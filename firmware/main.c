#include "device.h"

void System_Init(void)
{
    /* 하드 플로트 코드를 쓰기 전에 Cortex-M4 FPU를 활성화한다. */
    SCB->CPACR |= (0x3UL << 20) | (0x3UL << 22);

    Clock_Init();
    Timebase_Init();

    LED_Init();
    Motor_Init();
    US_Init();
    UART2_Init(115200U);

    Protocol_Init();
    State_Init();
}

void Main(void)
{
    System_Init();
    UART2_TXLine("READY PARKING");
    Protocol_SendState();

    for (;;)
    {
        Handler(); /* 폴링 핸들러 */
        State_Manager_Run(); /* 상태 -> 컨트롤러 */
    }
}
