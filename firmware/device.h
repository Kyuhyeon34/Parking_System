#ifndef DEVICE_H
#define DEVICE_H

#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"

#include "Timebase.h"
#include "LED_Controller.h"
#include "US_Controller.h"
#include "UART2_Controller.h"
#include "Motor_Controller.h"

#include "state_manager.h"
#include "protocol.h"
#include "Handler.h"

/* Motor_Handler가 차단기 통로 감지 과정에서 유지하는 상태값. */

typedef enum
{
    GATE_IDLE = 0,
    GATE_OPENING,
    GATE_HOLD,
    GATE_WAITING,
    GATE_CLOSING
} GateStage;

void Clock_Init(void);
void System_Init(void);

#endif
