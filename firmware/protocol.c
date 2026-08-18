#include <string.h>

#include "device.h"
#include "protocol.h"

#define COMMAND_BUFFER_SIZE 16U

extern char command_buffer[COMMAND_BUFFER_SIZE];
extern unsigned int command_length;
extern unsigned char command_overflow_flag;

void ProcessCommand(char *command)
{
    unsigned int index;

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

    if (strcmp(command, "RECOG START") == 0)
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
    else
    {
        UART2_TXLine("ERR UNKNOWN_COMMAND");
    }
}

void Protocol_Init(void)
{
    command_length = 0U;
    command_overflow_flag = 0U;
}

void Protocol_ReceiveByte(unsigned char byte)
{
    if (byte == '\r')
    {
        return;
    }

    if (byte == '\n')
    {
        if (command_overflow_flag)
        {
            UART2_TXLine("ERR LINE_TOO_LONG");
        }
        else
        {
            command_buffer[command_length] = '\0';
            ProcessCommand(command_buffer);
        }

        command_length = 0U;
        command_overflow_flag = 0U;
        return;
    }

    if (command_length < (COMMAND_BUFFER_SIZE - 1U))
    {
        command_buffer[command_length++] = (char)byte;
    }
    else
    {
        command_overflow_flag = 1U;
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
