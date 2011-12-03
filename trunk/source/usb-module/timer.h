#ifndef _TIMER_H_
#define _TIMER_H_ 

/* Macros */
#define udelay(t)	Timer_Sleep(t)
#define usleep(t)	Timer_Sleep(t)
#define msleep(t)	Timer_Sleep(t*1000)

/* Prototypes */
void Timer_Init(void);
void Timer_Sleep(u32 time);

#endif
