#include "device.h"
#include "US_Controller.h"

/* 소리는 1cm를 왕복하는 데 약 58us가 걸린다. */
#define US_US_PER_CM 58U

extern volatile unsigned int echo_start_us;
extern volatile unsigned char echo_active_flag;
extern volatile unsigned int distance_cm;
extern volatile unsigned char measure_done_flag;

void US_Init(void)
{
    Macro_Set_Bit(RCC->AHB1ENR, 1);   /* GPIOB 클럭 */
    Macro_Set_Bit(RCC->APB2ENR, 14);  /* SYSCFG 클럭, EXTI 매핑에 필요 */

    /* TRIG: 출력, 푸시풀, 풀업/다운 없음, 저속. */
    Macro_Write_Block(GPIOB->MODER, 0x3U, 0x1U, (US_TRIG_PIN * 2U));
    Macro_Clear_Bit(GPIOB->OTYPER, US_TRIG_PIN);
    Macro_Write_Block(GPIOB->PUPDR, 0x3U, 0x0U, (US_TRIG_PIN * 2U));

    /* ECHO: 입력, 풀다운이라 센서가 연결 안 됐으면 로우로 읽힌다. */
    Macro_Write_Block(GPIOB->MODER, 0x3U, 0x0U, (US_ECHO_PIN * 2U));
    Macro_Write_Block(GPIOB->PUPDR, 0x3U, 0x2U, (US_ECHO_PIN * 2U));

    GPIOB->BSRR = 1U << (US_TRIG_PIN + 16U);

    /* ECHO 핀을 해당 EXTI 라인에 연결하고 양쪽 엣지에서 인터럽트를 건다:
     * 상승 엣지에서 스톱워치를 시작하고, 하강 엣지에서 멈춘다. */
    Macro_Write_Block(SYSCFG->EXTICR[US_ECHO_PIN >> 2],
                      0xFU, US_ECHO_PORT, ((US_ECHO_PIN & 0x3U) * 4U));

    Macro_Set_Bit(EXTI->RTSR, US_ECHO_PIN);
    Macro_Set_Bit(EXTI->FTSR, US_ECHO_PIN);
    Macro_Set_Bit(EXTI->IMR, US_ECHO_PIN);

    echo_active_flag = 0U;
    measure_done_flag = 0U;
    distance_cm = US_DISTANCE_INVALID;

    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

void US_Trigger(void)
{
    /* 하강 엣지 짝 없이 상승 엣지만 남으면 스톱워치가 영원히 도니까,
     * 매번 새로 핑을 쏠 때마다 깨끗한 상태에서 시작한다. */
    echo_active_flag = 0U;

    GPIOB->BSRR = 1U << US_TRIG_PIN;
    Delay_us(10U);
    GPIOB->BSRR = 1U << (US_TRIG_PIN + 16U);
}

void US_Abort(void)
{
    echo_active_flag = 0U;
    measure_done_flag = 0U;
    distance_cm = US_DISTANCE_INVALID;
}

void US_OnEchoEdge(void)
{
    unsigned int now = Timebase_GetUs();

    if (Macro_Check_Bit_Set(GPIOB->IDR, US_ECHO_PIN))
    {
        echo_start_us = now;
        echo_active_flag = 1U;
    }
    else if (echo_active_flag != 0U)
    {
        unsigned int width = now - echo_start_us;

        echo_active_flag = 0U;

        if (width >= US_TIMEOUT_US)
        {
            distance_cm = US_DISTANCE_INVALID;
        }
        else
        {
            distance_cm = width / US_US_PER_CM;
        }

        measure_done_flag = 1U;
    }
}

int US_IsMeasureDone(void)
{
    if (measure_done_flag == 0U)
    {
        return 0;
    }

    measure_done_flag = 0U;
    return 1;
}

unsigned int US_GetDistanceCm(void)
{
    return distance_cm;
}
