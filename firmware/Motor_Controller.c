#include "device.h"
#include "Motor_Controller.h"

#define MOTOR_PWM_FREQUENCY 20000U
#define MOTOR_PWM_PERIOD    (TIMXCLK / MOTOR_PWM_FREQUENCY)
#define MOTOR_PWM_ARR       (MOTOR_PWM_PERIOD - 1U)

typedef enum
{
    GATE_IDLE = 0,
    GATE_OPENING,
    GATE_HOLD,
    GATE_WAITING,
    GATE_CLOSING
} GateStage;

static MotorDirection current_direction;
static unsigned int current_duty;

static GateStage gate_stage;
static unsigned int gate_started_tick;
static int gate_done;
static int gate_obstruction_timeout;

static unsigned int DutyToCompare(unsigned int duty_percent)
{
    return ((MOTOR_PWM_PERIOD * duty_percent) + 50U) / 100U;
}

void Motor_Init(void)
{
    Macro_Set_Bit(RCC->AHB1ENR, 0); /* GPIOA 클럭 */
    Macro_Set_Bit(RCC->APB1ENR, 3); /* TIM5 클럭 */

    /* PA0=TIM5_CH1, PA1=TIM5_CH2, 대체 기능 2번. */
    Macro_Write_Block(GPIOA->MODER, 0xFU, 0xAU, 0);
    Macro_Write_Block(GPIOA->AFR[0], 0xFFU, 0x22U, 0);
    Macro_Write_Block(GPIOA->PUPDR, 0xFU, 0x0U, 0);
    Macro_Write_Block(GPIOA->OSPEEDR, 0xFU, 0xAU, 0);

    TIM5->CR1 = 1U << 7;  /* 오토 리로드 프리로드 활성화. */
    TIM5->PSC = 0U;
    TIM5->ARR = MOTOR_PWM_ARR;
    TIM5->CCR1 = 0U;
    TIM5->CCR2 = 0U;

    /* 채널 1, 2 모두 PWM 모드 1 + 프리로드 활성화. */
    TIM5->CCMR1 = (6U << 4) | (1U << 3) |
                  (6U << 12) | (1U << 11);
    TIM5->CCER = (1U << 0) | (1U << 4);
    TIM5->EGR = 1U;
    TIM5->SR = 0U;
    TIM5->CR1 |= 1U;

    current_direction = MOTOR_STOP;
    current_duty = 0U;

    gate_stage = GATE_IDLE;
    gate_done = 0;
    gate_obstruction_timeout = 0;
}

void Motor_Set(MotorDirection direction, unsigned int duty_percent)
{
    unsigned int compare;

    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    if ((direction == MOTOR_STOP) || (duty_percent == 0U))
    {
        TIM5->CCR1 = 0U;
        TIM5->CCR2 = 0U;
        current_direction = MOTOR_STOP;
        current_duty = 0U;
        return;
    }

    compare = DutyToCompare(duty_percent);

    if ((current_direction != MOTOR_STOP) && (current_direction != direction))
    {
        /* H-브리지 방향을 바꾸기 전에 출력 0인 PWM 한 주기를 확정 반영한다. */
        TIM5->CCR1 = 0U;
        TIM5->CCR2 = 0U;
        TIM5->EGR = 1U;
    }

    if (direction == MOTOR_CW)
    {
        /* 실습 배선을 그대로 유지: CW는 PA1/TIM5_CH2로 구동한다. */
        TIM5->CCR1 = 0U;
        TIM5->CCR2 = compare;
    }
    else
    {
        TIM5->CCR2 = 0U;
        TIM5->CCR1 = compare;
    }

    current_direction = direction;
    current_duty = duty_percent;
}

void Motor_CW(unsigned int duty_percent)
{
    Motor_Set(MOTOR_CW, duty_percent);
}

void Motor_CCW(unsigned int duty_percent)
{
    Motor_Set(MOTOR_CCW, duty_percent);
}

void Motor_Stop(void)
{
    Motor_Set(MOTOR_STOP, 0U);
}

/* 차단기는 2초간 반시계로 돌아 팔을 들어올리고, 그다음 무엇을 판단하기
 * 전에 팔이 항상 한 박자 안정되도록 고정된 1초를 대기한 뒤, 차량이 센서
 * 아래 있는 동안은 계속 정지 상태를 유지한다. 경로가 비었다고 판정되면
 * 2초간 시계 방향으로 돌아 팔을 내린다. Gate_Update가 메인 루프에서 각
 * 단계를 진행시키므로 어디서도 블로킹되지 않는다. */
void Gate_Start(void)
{
    gate_stage = GATE_OPENING;
    gate_started_tick = Timebase_GetTick();
    gate_done = 0;
    gate_obstruction_timeout = 0;
    Motor_CCW(MOTOR_GATE_DUTY);
}

static void Gate_BeginClosing(void)
{
    gate_stage = GATE_CLOSING;
    gate_started_tick = Timebase_GetTick();
    Motor_CW(MOTOR_GATE_DUTY);
}

void Gate_Update(int path_blocked)
{
    switch (gate_stage)
    {
    case GATE_OPENING:
        if (Timebase_Elapsed(gate_started_tick, GATE_OPEN_MS))
        {
            gate_stage = GATE_HOLD;
            gate_started_tick = Timebase_GetTick();
            Motor_Stop();
        }
        break;

    case GATE_HOLD:
        /* 센서 판단이 개입하기 전 고정 대기 구간. 열림이 끝나는 순간
         * 마침 경로가 비어 있더라도 팔이 최소 한 박자는 반드시 정지해
         * 있도록 한다. */
        if (Timebase_Elapsed(gate_started_tick, GATE_HOLD_MS))
        {
            gate_stage = GATE_WAITING;
            gate_started_tick = Timebase_GetTick();
        }
        break;

    case GATE_WAITING:
        /* 차량이 지나가는 동안은 팔을 든 상태로 계속 유지한다. */
        if (!path_blocked)
        {
            Gate_BeginClosing();
        }
        else if (Timebase_Elapsed(gate_started_tick, GATE_WAIT_MAX_MS))
        {
            gate_obstruction_timeout = 1;
            Gate_BeginClosing();
        }
        break;

    case GATE_CLOSING:
        if (Timebase_Elapsed(gate_started_tick, GATE_CLOSE_MS))
        {
            gate_stage = GATE_IDLE;
            Motor_Stop();
            gate_done = 1;
        }
        break;

    case GATE_IDLE:
    default:
        break;
    }
}

int Gate_IsBusy(void)
{
    return (gate_stage != GATE_IDLE);
}

int Gate_IsWaitingForClear(void)
{
    return (gate_stage == GATE_WAITING);
}

int Gate_TakeDone(void)
{
    if (gate_done == 0)
    {
        return 0;
    }

    gate_done = 0;
    return 1;
}

int Gate_TakeObstructionTimeout(void)
{
    if (gate_obstruction_timeout == 0)
    {
        return 0;
    }

    gate_obstruction_timeout = 0;
    return 1;
}
