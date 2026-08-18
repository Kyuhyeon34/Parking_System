#ifndef UART2_CONTROLLER_H
#define UART2_CONTROLLER_H

void UART2_Init(unsigned int baud);
void UART2_TXByte(unsigned char byte);

int UART2_RX(unsigned char *byte);
void UART2_TX(const char *text);
void UART2_TXLine(const char *text);
void UART2_TXUnsigned(unsigned int value);

/* exception.c의 USART2_IRQHandler에서 호출된다. */
void UART2_PushRx(unsigned char byte);

#endif
