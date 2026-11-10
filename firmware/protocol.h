#ifndef PROTOCOL_H
#define PROTOCOL_H

void Protocol_Init(void);
void Protocol_ReceiveByte(unsigned char byte);
void ProcessCommand(char *command);

void Protocol_SendEvent(const char *event);
void Protocol_SendDistance(unsigned int cm);
void Protocol_SendState(void);

#endif
