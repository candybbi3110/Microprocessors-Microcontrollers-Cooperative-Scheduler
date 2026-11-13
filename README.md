# Lab 4 – Cooperative Scheduler (CO3010, HCMUT)

Cooperative scheduler on the **STM32F103C6**, built with STM32CubeIDE and simulated in Proteus.

- **Student:** Phạm Bùi Phương Trân – 2453286 – CC03
- **Course:** Microprocessors – Microcontrollers (CO3010)

## Features
- 10 ms tick from TIM2; `SCH_Update()` runs in **O(1)** (only the first task of a sorted list is checked)
- `RunMe` flag: the interrupt marks a task as due, the main loop runs it
- Periodic and one-shot tasks, `SCH_Add_Task()` returns a task ID, `SCH_Delete_Task()`
- Tasks due at the same tick run in order, with no drift
- Non-blocking `printf` on USART2 (115200 bit/s) through a ring buffer

| File | Content |
|---|---|
| `Src/scheduler.c`, `Inc/scheduler.h` | Scheduler |
| `Src/tasks.c`, `Inc/tasks.h` | Test tasks (5 LEDs, 10 ms task, one-shot tasks, delete) |
| `Src/main.c` | Initialisation, main loop, timer callback, UART buffer |
| `Lab4.ioc` | STM32CubeMX configuration |
| `Lab4.pdsprj`, `Lab4.hex` | Proteus project and compiled program |

## Demo
- LED1–LED5 toggle every 0.5 / 1 / 1.5 / 2 / 2.5 s
- The 10 ms task prints `get_time` and is deleted at 5 s
- One-shot tasks at 3000, 4200 and 7770 ms
