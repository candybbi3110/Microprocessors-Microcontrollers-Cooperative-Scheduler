#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_TICK_MS      10      /* TIM2 interrupt period (ms)              */
#define SCH_MAX_TASKS    16      /* size of the task pool                   */
#define NO_TASK_ID       0       /* returned by SCH_Add_Task when it fails  */

void     SCH_Init(void);
void     SCH_Update(void);                 /* call in the 10 ms timer ISR   */
void     SCH_Dispatch_Tasks(void);         /* call in while(1)              */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID); /* 1 = deleted, 0 = invalid ID   */
uint32_t SCH_Get_Time(void);               /* ms since SCH_Init()           */

/* DELAY and PERIOD are given in milliseconds and rounded down to whole
 * ticks. PERIOD = 0 means a one-shot task. */

#endif /* INC_SCHEDULER_H_ */
