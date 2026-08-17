#ifndef US_CONTROLLER_H
#define US_CONTROLLER_H

/* HC-SR04 배선. TRIG는 그냥 출력이고, ECHO는 EXTI 입력이다.
 *
 * ECHO를 다른 핀으로 옮기려면 아래 ECHO 관련 define 세 줄과 exception.c의
 * 해당 IRQ 핸들러 이름만 바꾸면 된다. PB6는 5V 내성이 있어서 대안으로
 * 쓰기 좋다. */
#define US_TRIG_PIN     0U /* PB0 */
#define US_ECHO_PIN     1U /* PB1 */
#define US_ECHO_PORT    1U /* B 포트에 대한 SYSCFG_EXTICR 코드 */

/* 약 4m를 넘어서면 센서가 응답하지 않는다. 25ms짜리 echo면 이미 4.3m다. */
#define US_TIMEOUT_US       25000U
#define US_DISTANCE_INVALID 9999U

void US_Init(void);
void US_Trigger(void);

/* 끝나지 않은 측정을 버린다. 스캔이 중단될 때 호출해서, 재개 시 새로운
 * 하강 엣지가 오래된 시작 시각과 잘못 짝지어지는 일을 막는다. */
void US_Abort(void);

/* ECHO EXTI 핸들러에서 호출된다. */
void US_OnEchoEdge(void);

/* 1회성 플래그: 측정이 완료될 때마다 딱 한 번만 1을 반환한다. */
int US_IsMeasureDone(void);

unsigned int US_GetDistanceCm(void);

#endif
