#include "device.h"
#include "UART2_Controller.h"

#define UART_RX_BUFFER_SIZE 32U
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1U)

extern volatile unsigned char rx_buffer[UART_RX_BUFFER_SIZE];
extern volatile unsigned int rx_head;
extern volatile unsigned int rx_tail;

void UART2_TXByte(unsigned char byte)
{
    while (!Macro_Check_Bit_Set(USART2->SR, 7))
    {
    }

    USART2->DR = byte;
}

void UART2_Init(unsigned int baud)
{
    unsigned int brr;

    rx_head = 0U;
    rx_tail = 0U;

    Macro_Set_Bit(RCC->AHB1ENR, 0);  /* GPIOA 클럭 */
    Macro_Set_Bit(RCC->APB1ENR, 17); /* USART2 클럭 */

    /* PA2=USART2_TX, PA3=USART2_RX, 대체 기능 7번. */
    Macro_Write_Block(GPIOA->MODER, 0xFU, 0xAU, 4);
    Macro_Write_Block(GPIOA->AFR[0], 0xFFU, 0x77U, 8);
    Macro_Write_Block(GPIOA->PUPDR, 0xFU, 0x5U, 4);
    Macro_Write_Block(GPIOA->OSPEEDR, 0xFU, 0xAU, 4);

    /* USART2는 APB1에 있어서, OVER8=0일 때 인코딩된 BRR 값은 PCLK1 / baud다. */
    brr = (PCLK1 + (baud / 2U)) / baud;
    USART2->BRR = brr;
    USART2->CR2 = 0U;
    USART2->CR3 = 0U;
    USART2->CR1 = (1U << 13) | /* USART 활성화 */
                  (1U << 5)  | /* 수신 레지스터 비어있지 않음 인터럽트 */
                  (1U << 3)  | /* 송신기 활성화 */
                  (1U << 2);   /* 수신기 활성화 */

    NVIC_ClearPendingIRQ(USART2_IRQn);
    NVIC_EnableIRQ(USART2_IRQn);
}

int UART2_RX(unsigned char *byte)
{
    unsigned int tail = rx_tail;

    if (tail == rx_head)
    {
        return 0;
    }

    *byte = rx_buffer[tail];
    rx_tail = (tail + 1U) & UART_RX_BUFFER_MASK;
    return 1;
}

void UART2_TX(const char *text)
{
    while (*text != '\0')
    {
        UART2_TXByte((unsigned char)*text++);
    }
}

void UART2_TXLine(const char *text)
{
    UART2_TX(text);
    UART2_TXByte('\r');
    UART2_TXByte('\n');
}

void UART2_TXUnsigned(unsigned int value)
{
    char digits[10];
    unsigned int count = 0U;

    do
    {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    while (value != 0U);

    while (count != 0U)
    {
        UART2_TXByte((unsigned char)digits[--count]);
    }
}

void UART2_PushRx(unsigned char byte)
{
    unsigned int next = (rx_head + 1U) & UART_RX_BUFFER_MASK;

    /* 호스트가 메인 루프보다 빠르게 보내면 블로킹하지 않고 바이트를 버린다. */
    if (next != rx_tail)
    {
        rx_buffer[rx_head] = byte;
        rx_head = next;
    }
}
