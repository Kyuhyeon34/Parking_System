#include "device.h"

/* US_Controller.c */
volatile unsigned int echo_start_us;
volatile unsigned char echo_active_flag;
volatile unsigned int distance_cm = US_DISTANCE_INVALID;
volatile unsigned char measure_done_flag;

/* UART2_Controller.c */
volatile unsigned char rx_buffer[32];
volatile unsigned int rx_head;
volatile unsigned int rx_tail;

/* Timebase.c */
volatile unsigned int tick_ms;

/* state_manager.c */
SystemState current_state;
unsigned char us_detected_flag;

/* protocol.c */
char command_buffer[16];
unsigned int command_length;
unsigned char command_overflow_flag;

/* Motor_Controller.c */
MotorDirection current_direction;
GateStage gate_stage;
unsigned int gate_started_tick;
unsigned char gate_done_flag;
unsigned char gate_timeout_flag;

/* Handler.c */
volatile unsigned char us_trigger_flag;
volatile unsigned char us_report_flag;
unsigned int gate_clear_count;
unsigned char gate_path_blocked_flag = 1U;
unsigned char gate_waiting_flag;
