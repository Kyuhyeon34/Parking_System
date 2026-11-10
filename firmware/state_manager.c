#include "device.h"
#include "state_manager.h"

extern SystemState current_state;
extern unsigned char us_detected_flag;

void State_Init(void)
{
    current_state = SYS_US_ON;
    us_detected_flag = 0U;
    Parking_LED(PARKING_LED_SCAN);
}

void State_Set(SystemState next)
{
    if (next == current_state)
    {
        return;
    }

    current_state = next;

    /* 처리 중 상태를 벗어난다는 건 다음 차량을 처음부터 다시 감지해야
     * 한다는 뜻이다. */
    if (next == SYS_US_ON)
    {
        us_detected_flag = 0U;
    }

    Protocol_SendState();
}

SystemState State_Get(void)
{
    return current_state;
}

const char *State_Name(SystemState state)
{
    switch (state)
    {
    case SYS_US_ON:
        return "US_ON";
    case SYS_US_OFF:
        return "US_OFF";
    case SYS_MOTOR:
        return "MOTOR";
    default:
        return "UNKNOWN";
    }
}

/* 현재 상태를 읽어서 컨트롤러들에게 무엇을 할지 지시한다. 이 시점에는
 * 각 핸들러가 이미 모든 인터럽트 플래그를 상태에 반영해 놓은 상태다. */
void State_Manager_Run(void)
{
    switch (current_state)
    {
    case SYS_US_ON:
        /* 스캔만 한다. 트리거 주기는 US_Handler가 관리한다. */
        break;

    case SYS_US_OFF:
        /* Qt가 다이얼로그를 끝내길 기다리는 중. 여기서 할 일은 없다. */
        break;

    case SYS_MOTOR:
        if (Gate_TakeObstructionTimeout())
        {
            Protocol_SendEvent("GATE OBSTRUCTED");
        }

        if (Gate_TakeDone())
        {
            Protocol_SendEvent("GATE DONE");
            State_Set(SYS_US_ON);
        }
        break;

    default:
        State_Set(SYS_US_ON);
        break;
    }
}

int State_IsVehiclePresent(void)
{
    return us_detected_flag;
}

void State_SetVehiclePresent(unsigned char present_flag)
{
    us_detected_flag = (unsigned char)(present_flag != 0U);
}
