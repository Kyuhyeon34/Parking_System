#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_CW,
    MOTOR_CCW
} MotorDirection;

/* 차단기 시퀀스: 2초 동안 열고, 최소 1초는 그대로 정지해 있다가,
 * 차량이 센서 아래에 있는 동안은 계속 정지 상태를 유지한 뒤, 2초 동안
 * 닫는다. */
#define GATE_OPEN_MS  2000U
#define GATE_HOLD_MS  1000U
#define GATE_CLOSE_MS 2000U

/* 이 거리 이내에 뭔가 있는 동안은 팔을 든 상태를 유지한다. 측정값이 이
 * 거리를 넘어서면 차량이 지나간 것으로 판단한다. */
#define GATE_CLEAR_CM 20U

/* 정지 유지 중에는 이 주기로 재측정하고, 노이즈성 샘플 하나로 팔이
 * 내려가지 않도록 이만큼 연속으로 클리어 판정이 나와야 닫는다. */
#define GATE_CHECK_PING_MS  60U
#define GATE_CLEAR_CONFIRM  3U

/* 순수 안전장치용. 차단기는 원래 차량이 필요한 만큼 기다려야 하지만,
 * 센서가 도중에 죽어버리면 팔이 영원히 올라간 채로 전체 상태머신이
 * 멈춰버리므로 이를 막기 위한 것. */
#define GATE_WAIT_MAX_MS 30000U

void Motor_Init(void);
void Motor_Set(MotorDirection direction);

void Motor_CW(void);
void Motor_CCW(void);
void Motor_Stop(void);

/* 논블로킹 차단기 시퀀스.
 *
 * Gate_Update는 센서를 직접 읽지 않고 "팔 아래에 뭔가 있는지"에 대한
 * 답을 인자로 받는다. 이렇게 해야 이 컨트롤러는 순수하게 기구 제어만
 * 담당하고 감지는 Motor_Handler에 맡길 수 있어서, 다른 컨트롤러들이
 * 서로를 모르는 구조와 일치한다. */
void Gate_Start(void);
void Gate_BeginClosing(void);
void Gate_Update(unsigned char path_blocked_flag);
int Gate_IsBusy(void);

/* 팔이 올라간 채로 차량이 빠지길 기다리는 동안만 참이며, 이때
 * Motor_Handler가 초음파 센서를 계속 핑해야 한다. */
int Gate_IsWaitingForClear(void);

/* 시퀀스가 끝나면 세팅되고, 읽으면 해제된다. */
int Gate_TakeDone(void);

/* 클리어된 경로가 아니라 타임아웃 때문에 차단기가 닫혔을 때 세팅된다. */
int Gate_TakeObstructionTimeout(void);

#endif
