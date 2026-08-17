#include "device.h"
#include "LED_Controller.h"

void LED_Init(void)
{
    Macro_Set_Bit(RCC->AHB1ENR, 0); /* GPIOA 클럭 */

    /* PA4/PA5: 출력, 푸시풀, 저속, 풀업/다운 없음. */
    Macro_Write_Block(GPIOA->MODER, 0xFU, 0x5U, 8);
    Macro_Clear_Bit(GPIOA->OTYPER, LED_RED_PIN);
    Macro_Clear_Bit(GPIOA->OTYPER, LED_GREEN_PIN);
    Macro_Write_Block(GPIOA->OSPEEDR, 0xFU, 0x0U, 8);
    Macro_Write_Block(GPIOA->PUPDR, 0xFU, 0x0U, 8);

    /* 외부 LED 둘 다 꺼진 채로 시작. BSRR 상위 절반이 출력 비트를 리셋한다. */
    GPIOA->BSRR = (1U << (LED_RED_PIN + 16U)) | (1U << (LED_GREEN_PIN + 16U));
}

void LED_ON_red(void)
{
    GPIOA->BSRR = 1U << LED_RED_PIN;
}

void LED_OFF_red(void)
{
    GPIOA->BSRR = 1U << (LED_RED_PIN + 16U);
}

void LED_ON_green(void)
{
    GPIOA->BSRR = 1U << LED_GREEN_PIN;
}

void LED_OFF_green(void)
{
    GPIOA->BSRR = 1U << (LED_GREEN_PIN + 16U);
}

/* 시스템 모드를 LED 한 쌍에 매핑하는 곳을 한 군데로 모아둔 것. 나중에
 * LED가 늘어나도 이 함수만 확장하면 된다. */
void Parking_LED(ParkingLedMode mode)
{
    if (mode == PARKING_LED_SCAN)
    {
        GPIOA->BSRR = (1U << LED_GREEN_PIN) | (1U << (LED_RED_PIN + 16U));
    }
    else
    {
        GPIOA->BSRR = (1U << LED_RED_PIN) | (1U << (LED_GREEN_PIN + 16U));
    }
}
