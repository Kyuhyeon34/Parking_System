#ifndef HANDLER_H
#define HANDLER_H

/* 메인 루프를 한 번 돌 때마다 호출되는 폴링 핸들러들. 각각 자기 플래그를
 * 확인해서 그 결과를 전역 시스템 상태에 반영한다. */
void Handler(void);
void US_Handler(void);
void UART2_Handler(void);
void Motor_Handler(void);
void LED_Handler(void);

#endif
