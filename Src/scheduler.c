/*
 * Cooperative scheduler with a RunMe flag and an O(1) SCH_Update().
 *
 * The tasks wait in a list sorted by due time. Each task stores its Delay
 * RELATIVE to the task in front of it:
 *
 *     head -> A(3) -> B(2) -> C(0)      A is due in 3 ticks, B in 5, C in 5.
 *
 * So SCH_Update() only looks at the FIRST task (no loop):
 *     Delay == 0 : the task is due     -> RunMe = 1 (dispatcher will run it)
 *     Delay != 0 : not yet             -> Delay--
 * SCH_Dispatch_Tasks() runs the first task while its RunMe flag is set.
 */
#include "main.h"
#include "scheduler.h"

#define NIL 0xFF                      /* "no task" / end of the list */

typedef struct {
    void   (*pTask)(void);            /* task function, 0 = free slot           */
    uint32_t Delay;                   /* ticks after the previous task in list  */
    uint32_t Period;                  /* ticks between runs, 0 = one-shot       */
    uint8_t  RunMe;                   /* 1 = due, the dispatcher must run it    */
    uint8_t  next;                    /* next task in the list                  */
} sTask;

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t head = NIL;                    /* first task of the list         */
static volatile uint32_t tick_count = 0;      /* ticks since SCH_Init()         */
static volatile uint32_t late_ticks = 0;      /* ticks that passed while the
                                                 first task was due but not run */

/* ---------------- helpers (interrupts must be disabled) ---------------- */

/* Put task 'id' in the list so that it is due 'delay' ticks from now.
 * Tasks due at the same time keep the order of their IDs. */
static void list_insert(uint8_t id, uint32_t delay)
{
    uint8_t prev = NIL, cur = head;
    while (cur != NIL && (SCH_tasks_G[cur].Delay < delay ||
                          (SCH_tasks_G[cur].Delay == delay && cur < id))) {
        delay -= SCH_tasks_G[cur].Delay;      /* pass the tasks due before me */
        prev = cur;
        cur  = SCH_tasks_G[cur].next;
    }
    SCH_tasks_G[id].Delay = delay;
    SCH_tasks_G[id].RunMe = 0;
    SCH_tasks_G[id].next  = cur;
    if (cur != NIL) SCH_tasks_G[cur].Delay -= delay;   /* task behind me waits less */
    if (prev == NIL) head = id; else SCH_tasks_G[prev].next = id;
}

/* Give the late ticks to the FIRST task only, then set its RunMe flag.
 * (Only the first task: a task is never pushed past its own due time,
 *  so periodic tasks do not drift.) */
static void catch_up(void)
{
    if (head == NIL) { late_ticks = 0; return; }
    if (SCH_tasks_G[head].Delay >= late_ticks) {
        SCH_tasks_G[head].Delay -= late_ticks;
        late_ticks = 0;
    } else {
        late_ticks -= SCH_tasks_G[head].Delay;
        SCH_tasks_G[head].Delay = 0;
    }
    if (SCH_tasks_G[head].Delay == 0) SCH_tasks_G[head].RunMe = 1;
    else                              SCH_tasks_G[head].RunMe = 0;
}

/* ---------------- interface ---------------- */

void SCH_Init(void)
{
    __disable_irq();
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask  = 0;
        SCH_tasks_G[i].Delay  = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe  = 0;
        SCH_tasks_G[i].next   = NIL;
    }
    head = NIL;
    tick_count = 0;
    late_ticks = 0;
    __enable_irq();
}

/* Timer interrupt, every SCH_TICK_MS. O(1): only the first task is checked. */
void SCH_Update(void)
{
    tick_count++;
    if (head == NIL) return;

    if (SCH_tasks_G[head].Delay == 0) {
        /* already due, the dispatcher has not run it yet */
        SCH_tasks_G[head].RunMe = 1;
        late_ticks++;
    } else {
        SCH_tasks_G[head].Delay--;
        if (SCH_tasks_G[head].Delay == 0) {
            SCH_tasks_G[head].RunMe = 1;      /* due now */
        }
    }
}

/* Main loop: run every task whose RunMe flag is set, in order. */
void SCH_Dispatch_Tasks(void)
{
    while (1) {
        __disable_irq();
        catch_up();                            /* first task due? -> RunMe */
        if (head == NIL || SCH_tasks_G[head].RunMe == 0) {
            __enable_irq();
            return;                            /* nothing to run */
        }

        uint8_t id = head;                     /* take the first task out */
        void (*task)(void) = SCH_tasks_G[id].pTask;
        head = SCH_tasks_G[id].next;
        SCH_tasks_G[id].RunMe = 0;

        if (SCH_tasks_G[id].Period == 0) {
            SCH_tasks_G[id].pTask = 0;         /* one-shot: delete it */
            SCH_tasks_G[id].next  = NIL;
        } else {
            list_insert(id, SCH_tasks_G[id].Period);  /* periodic: next run */
        }
        __enable_irq();

        task();                                /* run with interrupts on */
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD)
{
    if (pFunction == 0) return NO_TASK_ID;

    __disable_irq();
    uint8_t id = NIL;
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {      /* find a free slot */
        if (SCH_tasks_G[i].pTask == 0) { id = i; break; }
    }
    if (id == NIL) {                                   /* array is full */
        __enable_irq();
        return NO_TASK_ID;
    }
    SCH_tasks_G[id].pTask  = pFunction;
    SCH_tasks_G[id].Period = PERIOD / SCH_TICK_MS;     /* ms -> ticks */
    if (PERIOD > 0 && SCH_tasks_G[id].Period == 0) SCH_tasks_G[id].Period = 1;

    list_insert(id, DELAY / SCH_TICK_MS + late_ticks);
    catch_up();
    __enable_irq();

    return (uint32_t)id + 1;                           /* ID 0 = error */
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == NO_TASK_ID || taskID > SCH_MAX_TASKS) return 0;
    uint8_t id = (uint8_t)(taskID - 1);

    __disable_irq();
    if (SCH_tasks_G[id].pTask == 0) {                  /* already deleted */
        __enable_irq();
        return 0;
    }
    uint8_t prev = NIL, cur = head;                    /* find it in the list */
    while (cur != NIL && cur != id) { prev = cur; cur = SCH_tasks_G[cur].next; }
    if (cur == id) {
        uint8_t nxt = SCH_tasks_G[id].next;
        if (nxt != NIL) SCH_tasks_G[nxt].Delay += SCH_tasks_G[id].Delay;
        if (prev == NIL) head = nxt; else SCH_tasks_G[prev].next = nxt;
    }
    SCH_tasks_G[id].pTask  = 0;
    SCH_tasks_G[id].Period = 0;
    SCH_tasks_G[id].RunMe  = 0;
    SCH_tasks_G[id].next   = NIL;
    catch_up();
    __enable_irq();
    return 1;
}

uint32_t SCH_Get_Time(void)
{
    return tick_count * SCH_TICK_MS;
}
