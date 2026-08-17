#include "device.h"
#include "Timebase.h"

/* TIM2는 TIMXCLK로 클럭되는 32비트 타이머다. 이를 1MHz로 프리스케일하면
 * 약 71분마다 한 번 랩어라운드되는 프리런 마이크로초 카운터가 되는데,
 * 초음파 echo 측정에서는 신경 쓸 필요 없을 만큼 충분히 긴 주기다. */
#define TIMEBASE_US_PRESCALER ((TIMXCLK / 1000000U) - 1U)

static volatile unsigned int tick_ms;

void Timebase_Init(void)
{
    tick_ms = 0U;

    /* TIM2 프리런 마이크로초 카운터. */
    Macro_Set_Bit(RCC->APB1ENR, 0); /* TIM2 클럭 */

    TIM2->CR1 = 0U;
    TIM2->PSC = TIMEBASE_US_PRESCALER;
    TIM2->ARR = 0xFFFFFFFFU;
    TIM2->CNT = 0U;
    TIM2->EGR = 1U; /* 프리스케일러 값을 즉시 로드. */
    TIM2->SR = 0U;
    TIM2->CR1 = 1U; /* 카운터 활성화, 인터럽트 없음. */

    /* SysTick을 1ms 시스템 틱으로 사용. */
    SysTick->LOAD = (HCLK / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = (1U << 2) | /* 프로세서 클럭 */
                    (1U << 1) | /* 틱 인터럽트 */
                    (1U << 0);  /* 활성화 */
}

unsigned int Timebase_GetTick(void)
{
    return tick_ms;
}

unsigned int Timebase_GetUs(void)
{
    return TIM2->CNT;
}

void Delay_us(unsigned int us)
{
    unsigned int start = TIM2->CNT;

    while ((TIM2->CNT - start) < us)
    {
    }
}

/* 부호 없는 뺄셈이라 32비트 밀리초 틱이 랩어라운드돼도 안전하게 동작한다. */
int Timebase_Elapsed(unsigned int start_tick, unsigned int ms)
{
    return ((tick_ms - start_tick) >= ms);
}

void Timebase_Tick(void)
{
    tick_ms++;
}
