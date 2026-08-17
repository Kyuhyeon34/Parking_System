#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

/* 설계 노트에 있던 세 가지 시스템 상태.
 *
 *   SYS_US_ON  : 초음파 센서가 스캔 중          -> 초록 LED
 *   SYS_US_OFF : Qt가 인식 중 / 다이얼로그 표시 중 -> 빨강 LED
 *   SYS_MOTOR  : 차단기 시퀀스 진행 중           -> 빨강 LED
 *
 * 센서가 꺼져 있는 동안은 어차피 새 차량이 들어올 수 없으므로, 두 처리
 * 중 상태 모두 스캔을 중단한다. */
typedef enum
{
    SYS_US_ON = 0,
    SYS_US_OFF,
    SYS_MOTOR
} SystemState;

/* 이 거리보다 가까워지면 인식 흐름을 시작한다. */
#define US_DETECT_CM 10U

/* 임계값 경계에 딱 걸쳐 주차된 차량 때문에 감지가 들쭉날쭉하지 않도록
 * 두는 히스테리시스. */
#define US_CLEAR_CM 15U

void State_Init(void);
void State_Set(SystemState next);
SystemState State_Get(void);
const char *State_Name(SystemState state);

void State_Manager_Run(void);

/* US_Handler가 세팅해서, 차량당 감지 이벤트가 한 번만 보고되게 한다. */
int State_IsVehiclePresent(void);
void State_SetVehiclePresent(int present);

#endif
