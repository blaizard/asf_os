#include "board.h"
#include "pm.h"
#include "delay.h"
#include "gpio.h"
#include "os_core.h"

#define CPU_HZ  FOSC0

struct os_task task_1, task_2, task_3, task_4;

struct task_args {
	uint32_t delay_ms;
	uint32_t pin;
};

void task(void *raw_args)
{
	struct task_args *args = (struct task_args *) raw_args;
	gpio_set_gpio_pin(args->pin);
	while (1) {
		gpio_tgl_gpio_pin(args->pin);
		os_task_delay(OS_MS_TO_TICK(args->delay_ms));
		os_task_yield();
	};
}

void task2(void *args)
{
	while (1) {
		os_task_yield();
	};
}

int main(void)
{
	struct task_args args_1 = {
		.delay_ms = 100,
		.pin = LED0_GPIO
	};
	struct task_args args_2 = {
		.delay_ms = 1000,
		.pin = LED1_GPIO
	};
	struct task_args args_3 = {
		.delay_ms = 1000,
		.pin = LED2_GPIO
	};

	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	os_task_add(&task_1, task, &args_1, 500, OS_TASK_DEFAULT);
	os_task_add(&task_2, task, &args_2, 500, OS_TASK_DEFAULT);
	os_task_add(&task_3, task, &args_3, 500, OS_TASK_DEFAULT);
	os_task_add(&task_4, task2, &task_1, 500, OS_TASK_DEFAULT);

	os_start(CPU_HZ);
}
