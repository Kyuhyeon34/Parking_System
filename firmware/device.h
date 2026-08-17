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

void Clock_Init(void);

#endif
