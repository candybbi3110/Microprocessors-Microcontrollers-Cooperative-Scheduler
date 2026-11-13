/* Demo tasks for Lab 4 */
#include <stdio.h>
#include "main.h"
#include "scheduler.h"
#include "tasks.h"

/* 1 = the 10 ms task prints the time every 10 ms (required by the demo).
 * It is deleted automatically at t = 5 s so that the other messages can be
 * read on the terminal; this also demonstrates SCH_Delete_Task(). */
#define PRINT_EVERY_10MS   1
#define STOP_10MS_AT       5000

static uint32_t id_tick10 = NO_TASK_ID;

/* ---------- five periodic LED tasks (Problem, page 22) --------------- */
static void Task_LED1(void) { HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); printf("[%6lu ms] LED1 (0.5 s)\r\n", (unsigned long)SCH_Get_Time()); }
static void Task_LED2(void) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); printf("[%6lu ms] LED2 (1.0 s)\r\n", (unsigned long)SCH_Get_Time()); }
static void Task_LED3(void) { HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); printf("[%6lu ms] LED3 (1.5 s)\r\n", (unsigned long)SCH_Get_Time()); }
static void Task_LED4(void) { HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin); printf("[%6lu ms] LED4 (2.0 s)\r\n", (unsigned long)SCH_Get_Time()); }
static void Task_LED5(void) { HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin); printf("[%6lu ms] LED5 (2.5 s)\r\n", (unsigned long)SCH_Get_Time()); }

/* ---------- 10 ms callback: print get_time (Demonstration) ----------- */
static void Task_Tick10ms(void)
{
#if PRINT_EVERY_10MS
    printf("t=%lu\r\n", (unsigned long)SCH_Get_Time());
#endif
}

/* ---------- one-shot tasks registered before the main loop ----------- */
static void OneShot_A(void) { printf("[%6lu ms] one-shot A (delay 3000 ms)\r\n", (unsigned long)SCH_Get_Time()); }
static void OneShot_B(void) { printf("[%6lu ms] one-shot B (delay 4200 ms)\r\n", (unsigned long)SCH_Get_Time()); }
static void OneShot_C(void) { printf("[%6lu ms] one-shot C (delay 7770 ms)\r\n", (unsigned long)SCH_Get_Time()); }

static void OneShot_Stop10ms(void)
{
    if (SCH_Delete_Task(id_tick10))
        printf("[%6lu ms] SCH_Delete_Task: 10 ms task deleted\r\n", (unsigned long)SCH_Get_Time());
}

void Tasks_Setup(void)
{
    SCH_Add_Task(Task_LED1, 500,  500);
    SCH_Add_Task(Task_LED2, 1000, 1000);
    SCH_Add_Task(Task_LED3, 1500, 1500);
    SCH_Add_Task(Task_LED4, 2000, 2000);
    SCH_Add_Task(Task_LED5, 2500, 2500);

    id_tick10 = SCH_Add_Task(Task_Tick10ms, 10, 10);

    SCH_Add_Task(OneShot_A, 3000, 0);
    SCH_Add_Task(OneShot_B, 4200, 0);
    SCH_Add_Task(OneShot_C, 7770, 0);
    SCH_Add_Task(OneShot_Stop10ms, STOP_10MS_AT, 0);
}
