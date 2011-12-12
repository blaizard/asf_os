#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "os_core.h"
#include "cycle_counter.h"

#define CPU_HZ FOSC0

#define ACTIVITY_TIME_MS 20000

volatile static uint32_t activity_ref, activity_task1, activity_task2, activity_task3;
void calculate_task_swtiching_time(void);

static inline uint32_t task_activity(void) {
	uint32_t activity = 0;
	t_cpu_time timeout;

	cpu_set_timeout(cpu_ms_2_cy(ACTIVITY_TIME_MS, CPU_HZ), &timeout);
	while (!cpu_is_timeout(&timeout)) {
		activity++;
	}
	return activity;
}

void task1(void *args)
{
	activity_task1 = task_activity();
	while (true) {
		calculate_task_swtiching_time();
	}
}

void task2(void *args)
{
	activity_task2 = task_activity();
	while (true) {
	}
}

void task3(void *args)
{
	activity_task2 = task_activity();
	while (true) {
	}
}

void calculate_task_swtiching_time(void)
{
	const int nb_tasks = 3;
	volatile uint32_t task_switching_time;

	task_switching_time = activity_ref;
	// Get the time overhead due to task switching
	task_switching_time -= activity_task1 + activity_task2 + activity_task3;
	// Divide this time by the number of task switching
	task_switching_time /= (ACTIVITY_TIME_MS * CONFIG_OS_TICK_HZ) / 1000;
	// Convert this number representing the activity to a time
	task_switching_time = (task_switching_time * ACTIVITY_TIME_MS) / activity_ref;
}

int main(void)
{
	struct os_task task_1, task_2, task_3;

	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	os_task_create(&task_1, task1, NULL, 200, OS_TASK_DEFAULT);
	os_task_create(&task_2, task2, NULL, 200, OS_TASK_DEFAULT);
	os_task_create(&task_3, task3, NULL, 200, OS_TASK_DEFAULT);

	// Get the activity reference. It is a number reflectiing the number of
	// cycles used to process this function. This number will be used as a
	// reference to determine the task switching time.
	activity_ref = task_activity();

	// Start the task switching
	os_start(CPU_HZ);
}
