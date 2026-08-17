#ifndef HANDLER_H
#define HANDLER_H

/* SysTick_Handler(exception.c)가 자신의 주기에 맞춰 세팅하고, 아래
 * US_Handler가 소비한다. ISR이 이제 다른 모든 인터럽트와 함께
 * exception.c에 있고, 이를 처리하는 폴링 핸들러는 여기 남아 있어서
 * extern으로 선언해둔다. */
extern volatile unsigned int us_trigger_pending;
extern volatile unsigned int us_report_pending;

/* 메인 루프를 한 번 돌 때마다 호출되는 폴링 핸들러들. 각각 자기 플래그를
 * 확인해서 그 결과를 전역 시스템 상태에 반영한다. */
void US_Handler(void);
void UART2_Handler(void);
void Motor_Handler(void);
void LED_Handler(void);

#endif
