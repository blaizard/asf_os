#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "os_core.h"

#define CPU_HZ  FOSC0

struct os_task task_1, task_2, task_3, task_4;

static struct os_interrupt int_1;
static struct os_semaphore sem;
static struct os_mutex mutex;
static struct os_event event;

struct task_args {
	uint32_t delay_ms;
	uint32_t pin;
};

void task2(void *args);
void led_on();
void led_blink(void *raw_args)
{
	struct task_args *args = (struct task_args *) raw_args;

	while (1) {
		os_task_sleep(os_task_get_current(), &event);
		gpio_tgl_gpio_pin(args->pin);
		os_task_delay(OS_MS_TO_TICK(args->delay_ms));
		//os_semaphore_release(&sem);
		os_mutex_unlock(&mutex);
	}
}

void task2(void *args)
{
	volatile static uint32_t task_switch;
	volatile static uint32_t jitter;
	while (1) {
		gpio_tgl_gpio_pin(LED3_GPIO);
		os_task_delay(OS_MS_TO_TICK(500));
		task_switch = os_statistics_get_task_switch_time();
		jitter = os_statistics_get_task_switch_time_jitter();
	}
}

void led_on()
{
	gpio_clr_gpio_pin(LED3_GPIO);
}

int main(void)
{
	struct task_args args_1 = {
		.delay_ms = 100,
		.pin = LED0_GPIO
	};
	struct task_args args_2 = {
		.delay_ms = 100,
		.pin = LED1_GPIO
	};
	struct task_args args_3 = {
		.delay_ms = 100,
		.pin = LED2_GPIO
	};
	struct task_args args_4 = {
		.delay_ms = 100,
		.pin = LED3_GPIO
	};

	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	os_semaphore_create(&sem, 2, 2);
	//os_semaphore_create_event(&event, &sem);

	os_mutex_create(&mutex);
	os_mutex_create_event(&event, &mutex);

	os_interrupt_setup(&int_1, task2, NULL);

	os_task_create(&task_1, led_blink, &args_1, 500, OS_TASK_DEFAULT);
	os_task_create(&task_2, led_blink, &args_2, 500, OS_TASK_DEFAULT);
	os_task_create(&task_3, led_blink, &args_3, 500, OS_TASK_DEFAULT);
	os_task_create(&task_4, led_blink, &args_4, 500, OS_TASK_DEFAULT);
	//os_task_create(&task_4, task2, NULL, 500, OS_TASK_DEFAULT);

	//os_task_set_priority(&task_2, OS_PRIORITY_2);

	os_start(CPU_HZ);
}
