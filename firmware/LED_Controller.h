#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#define LED_RED_PIN   4U /* PA4 */
#define LED_GREEN_PIN 5U /* PA5 */

typedef enum
{
    PARKING_LED_SCAN = 0, /* 초록 켜짐 - 초음파 센서가 스캔 중 */
    PARKING_LED_BUSY      /* 빨강 켜짐 - 인식/입출차/차단기 동작 중 */
} ParkingLedMode;

void LED_Init(void);

void LED_ON_red(void);
void LED_OFF_red(void);
void LED_ON_green(void);
void LED_OFF_green(void);

void Parking_LED(ParkingLedMode mode);

#endif
