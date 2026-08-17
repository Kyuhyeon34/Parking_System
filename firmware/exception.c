#include "device.h"

/* HC-SR04는 이전 펄스가 다 사그라들 때까지 핑 사이에 약 60ms가 필요하다.
 * 거리는 측정하는 주기보다 덜 자주 호스트에 보고된다. */
#define US_TRIGGER_PERIOD_MS 60U
#define US_REPORT_PERIOD_MS  100U

void _Invalid_ISR(void)
{
    __disable_irq();
    for (;;)
    {
        /* 디버거로 어떤 예외에서 걸렸는지 확인할 수 있도록 여기서 멈춘다. */
    }
}

/* ------------------------------------------------------------------ */
/* 인터럽트 서비스 루틴                                                 */
/*                                                                     */
/* crt0.s가 모든 벡터를 Invalid_ISR에 대한 약한 별칭으로 선언해두기      */
/* 때문에, 여기서 핸들러를 정의하기만 하면 벡터 테이블을 건드리지       */
/* 않고도 자동으로 덮어써진다.                                         */
/* ------------------------------------------------------------------ */

void SysTick_Handler(void)
{
    static unsigned int trigger_countdown;
    static unsigned int report_countdown;

    Timebase_Tick();

    trigger_countdown++;
    if (trigger_countdown >= US_TRIGGER_PERIOD_MS)
    {
        trigger_countdown = 0U;
        us_trigger_pending = 1U;
    }

    report_countdown++;
    if (report_countdown >= US_REPORT_PERIOD_MS)
    {
        report_countdown = 0U;
        us_report_pending = 1U;
    }
}

void EXTI1_IRQHandler(void)
{
    if (Macro_Check_Bit_Set(EXTI->PR, US_ECHO_PIN))
    {
        /* 펜딩 비트는 1을 써서 지운다. */
        EXTI->PR = 1U << US_ECHO_PIN;
        US_OnEchoEdge();
    }
}

void USART2_IRQHandler(void)
{
    unsigned int status = USART2->SR;

    if ((status & ((1U << 5) | (1U << 3) | (1U << 2) | (1U << 1))) != 0U)
    {
        /* DR을 읽으면 RXNE와 에러 플래그가 함께 지워진다. */
        unsigned char byte = (unsigned char)USART2->DR;

        if ((status & (1U << 5)) != 0U)
        {
            UART2_PushRx(byte);
        }
    }
}
