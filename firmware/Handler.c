#include "device.h"
#include "Handler.h"

/* 감지/해제를 알리기 전에 필요한 연속 일치 샘플 수. 60ms 간격으로 3번이면
 * 5분의 1초도 안 걸려서 즉각적으로 느껴지면서도 단일 샘플 노이즈는
 * 걸러낸다. */
#define US_CONFIRM_COUNT 3U

/* SysTick_Handler(exception.c)가 자신의 주기에 맞춰 세팅하고, 여기서
 * 소비한다. ISR과 이 폴링 핸들러가 이제 서로 다른 파일에 있어서
 * Handler.h에 extern으로 선언해둔다. */
volatile unsigned int us_trigger_pending;
volatile unsigned int us_report_pending;

/* ------------------------------------------------------------------ */
/* 폴링 핸들러                                                          */
/* ------------------------------------------------------------------ */

void US_Handler(void)
{
    static int suspended;
    static unsigned int near_count;
    static unsigned int far_count;

    unsigned int distance;

    /* 다이얼로그나 차단기가 시스템을 점유 중일 땐 핑을 쏴봐야 의미가
     * 없다: 어차피 새 차량이 차단기를 통과할 수 없다. */
    if (State_Get() != SYS_US_ON)
    {
        us_trigger_pending = 0U;
        us_report_pending = 0U;

        /* 반쯤 측정하다 만 값은 버려서, 재개했을 때 깨끗하게 시작하도록 한다. */
        if (!suspended)
        {
            suspended = 1;
            near_count = 0U;
            far_count = 0U;
            US_Abort();
        }
        return;
    }

    suspended = 0;

    if (us_trigger_pending != 0U)
    {
        us_trigger_pending = 0U;
        US_Trigger();
    }

    if (!US_IsMeasureDone())
    {
        return;
    }

    distance = US_GetDistanceCm();

    if (us_report_pending != 0U)
    {
        us_report_pending = 0U;
        Protocol_SendDistance(distance);
    }

    /* HC-SR04 측정값은 흔들리고 가끔 터무니없는 값도 나오므로, 샘플 하나로는
     * 절대 차단기를 열기에 충분하지 않다. 같은 답이 US_CONFIRM_COUNT번
     * 연속으로 나왔을 때만 보고한다. */
    if (!State_IsVehiclePresent())
    {
        if (distance <= US_DETECT_CM)
        {
            near_count++;
            far_count = 0U;
        }
        else
        {
            near_count = 0U;
        }

        if (near_count >= US_CONFIRM_COUNT)
        {
            near_count = 0U;
            State_SetVehiclePresent(1);
            UART2_TX("EVT DETECT ");
            UART2_TXUnsigned(distance);
            UART2_TXLine("");
        }
    }
    else
    {
        if (distance >= US_CLEAR_CM)
        {
            far_count++;
            near_count = 0U;
        }
        else
        {
            far_count = 0U;
        }

        if (far_count >= US_CONFIRM_COUNT)
        {
            far_count = 0U;
            State_SetVehiclePresent(0);
            Protocol_SendEvent("CLEAR");
        }
    }
}

void UART2_Handler(void)
{
    unsigned char byte;

    while (UART2_RX(&byte))
    {
        Protocol_ReceiveByte(byte);
    }
}

void Motor_Handler(void)
{
    static unsigned int check_ping_tick;
    static unsigned int clear_count;
    static int path_blocked = 1;
    static int was_waiting;

    const int waiting = Gate_IsWaitingForClear();

    if (waiting)
    {
        if (!was_waiting)
        {
            /* 대기 상태 진입. "막혀 있음"에서 시작한다: 응답이 아예 없는
             * 센서를 클리어한 경로로 오해하면 안 되기 때문이다.
             * Gate_Update의 타임아웃이 이 가정 때문에 차단기가 영원히
             * 멈추는 걸 막아준다. */
            was_waiting = 1;
            path_blocked = 1;
            clear_count = 0;
            check_ping_tick = Timebase_GetTick();
            US_Abort();
        }

        /* SYS_MOTOR 상태에서는 US_Handler가 쉬고 있으므로, 여기서
         * 센서를 서로 충돌 없이 직접 구동할 수 있다. */
        if (Timebase_Elapsed(check_ping_tick, GATE_CHECK_PING_MS))
        {
            check_ping_tick = Timebase_GetTick();
            US_Trigger();
        }

        if (US_IsMeasureDone())
        {
            if (US_GetDistanceCm() > GATE_CLEAR_CM)
            {
                clear_count++;
                if (clear_count >= GATE_CLEAR_CONFIRM)
                {
                    path_blocked = 0;
                }
            }
            else
            {
                clear_count = 0;
                path_blocked = 1;
            }
        }
    }
    else if (was_waiting)
    {
        /* 대기 상태를 빠져나갈 때만 실행되고, 매 루프마다 실행되지는
         * 않는다: US_Abort는 측정 플래그를 지워버리므로, 여기서 무조건
         * 호출하면 US_Handler가 값을 보기도 전에 매번 측정값을 날리게
         * 된다. */
        was_waiting = 0;
        path_blocked = 1;
        clear_count = 0;
        US_Abort();
    }

    Gate_Update(path_blocked);
}

void LED_Handler(void)
{
    if (State_Get() == SYS_US_ON)
    {
        Parking_LED(PARKING_LED_SCAN);
    }
    else
    {
        Parking_LED(PARKING_LED_BUSY);
    }
}
