#include <string.h>

#include "device.h"
#include "protocol.h"

#define COMMAND_BUFFER_SIZE 64U

static char command_buffer[COMMAND_BUFFER_SIZE];
static unsigned int command_length;
static int command_overflow;

static int ParseDuty(const char *text, unsigned int *duty)
{
    unsigned int value = 0U;

    if (*text == '\0')
    {
        return 0;
    }

    while (*text != '\0')
    {
        if ((*text < '0') || (*text > '9'))
        {
            return 0;
        }

        value = (value * 10U) + (unsigned int)(*text - '0');

        if (value > 100U)
        {
            return 0;
        }

        text++;
    }

    *duty = value;
    return 1;
}

static void SendOkDistance(unsigned int cm)
{
    UART2_TX("OK DIST ");
    UART2_TXUnsigned(cm);
    UART2_TXLine("");
}

static void ProcessCommand(char *command)
{
    unsigned int index;
    unsigned int duty;

    for (index = 0U; command[index] != '\0'; index++)
    {
        if ((command[index] >= 'a') && (command[index] <= 'z'))
        {
            command[index] = (char)(command[index] - ('a' - 'A'));
        }
    }

    if (command[0] == '\0')
    {
        return;
    }

    if (strcmp(command, "PING") == 0)
    {
        UART2_TXLine("OK PONG");
    }
    else if (strcmp(command, "STATE?") == 0)
    {
        UART2_TX("OK STATE ");
        UART2_TXLine(State_Name(State_Get()));
    }
    else if (strcmp(command, "DIST?") == 0)
    {
        SendOkDistance(US_GetDistanceCm());
    }
    else if (strcmp(command, "RECOG START") == 0)
    {
        /* The dialog is up, so stop scanning and turn the LED red. */
        State_Set(SYS_US_OFF);
        UART2_TXLine("OK RECOG");
    }
    else if (strcmp(command, "RECOG CANCEL") == 0)
    {
        if (State_Get() == SYS_US_OFF)
        {
            State_Set(SYS_US_ON);
        }

        UART2_TXLine("OK SCAN");
    }
    else if (strcmp(command, "GATE OPEN") == 0)
    {
        if (Gate_IsBusy())
        {
            UART2_TXLine("ERR GATE_BUSY");
        }
        else
        {
            State_Set(SYS_MOTOR);
            Gate_Start();
            UART2_TXLine("OK GATE");
        }
    }
    else if (strcmp(command, "LED RED ON") == 0)
    {
        LED_ON_red();
        UART2_TXLine("OK LED");
    }
    else if (strcmp(command, "LED RED OFF") == 0)
    {
        LED_OFF_red();
        UART2_TXLine("OK LED");
    }
    else if (strcmp(command, "LED GREEN ON") == 0)
    {
        LED_ON_green();
        UART2_TXLine("OK LED");
    }
    else if (strcmp(command, "LED GREEN OFF") == 0)
    {
        LED_OFF_green();
        UART2_TXLine("OK LED");
    }
    else if (strcmp(command, "MOTOR STOP") == 0)
    {
        Motor_Stop();
        UART2_TXLine("OK MOTOR STOP");
    }
    else if (strncmp(command, "MOTOR CW ", 9) == 0)
    {
        if (ParseDuty(&command[9], &duty))
        {
            Motor_CW(duty);
            UART2_TXLine("OK MOTOR CW");
        }
        else
        {
            UART2_TXLine("ERR MOTOR_DUTY");
        }
    }
    else if (strncmp(command, "MOTOR CCW ", 10) == 0)
    {
        if (ParseDuty(&command[10], &duty))
        {
            Motor_CCW(duty);
            UART2_TXLine("OK MOTOR CCW");
        }
        else
        {
            UART2_TXLine("ERR MOTOR_DUTY");
        }
    }
    else
    {
        UART2_TXLine("ERR UNKNOWN_COMMAND");
    }
}

void Protocol_Init(void)
{
    command_length = 0U;
    command_overflow = 0;
}

void Protocol_ReceiveByte(unsigned char byte)
{
    if (byte == '\r')
    {
        return;
    }

    if (byte == '\n')
    {
        if (command_overflow)
        {
            UART2_TXLine("ERR LINE_TOO_LONG");
        }
        else
        {
            command_buffer[command_length] = '\0';
            ProcessCommand(command_buffer);
        }

        command_length = 0U;
        command_overflow = 0;
        return;
    }

    if (command_length < (COMMAND_BUFFER_SIZE - 1U))
    {
        command_buffer[command_length++] = (char)byte;
    }
    else
    {
        command_overflow = 1;
    }
}

void Protocol_SendEvent(const char *event)
{
    UART2_TX("EVT ");
    UART2_TXLine(event);
}

void Protocol_SendDistance(unsigned int cm)
{
    UART2_TX("DIST ");
    UART2_TXUnsigned(cm);
    UART2_TXLine("");
}

void Protocol_SendState(void)
{
    UART2_TX("STATE ");
    UART2_TXLine(State_Name(State_Get()));
}
