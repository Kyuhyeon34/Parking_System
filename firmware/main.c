#include "device.h"

static void System_Init(void)
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
        UART2_Handler();     /* 호스트 명령 처리 */
        US_Handler();        /* 거리 측정 및 감지 */
        Motor_Handler();     /* 차단기 시퀀스 */
        State_Manager_Run(); /* 상태 -> 컨트롤러 */
        LED_Handler();       /* 상태 -> LED */
    }
}
