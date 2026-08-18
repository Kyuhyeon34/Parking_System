#ifndef TIMEBASE_H
#define TIMEBASE_H

void Timebase_Init(void);

unsigned int Timebase_GetTick(void);
unsigned int Timebase_GetUs(void);

void Delay_us(unsigned int us);

int Timebase_Elapsed(unsigned int start_tick, unsigned int ms);

void Timebase_Tick(void);

#endif
