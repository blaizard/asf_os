/*! \file
 * \brief Mutex example
 * \author Blaise Lengrand (blaise.lengrand@gmail.com)
 * \version 0.1
 * \date 2011
 *
 * \section eeos_license License
 * \ref group_os is provided in source form for FREE evaluation, for
 * educational use or for peaceful research. If you plan on using \ref group_os
 * in a commercial product you need to contact the author to properly license
 * its use in your product. The fact that the  source is provided does
 * NOT mean that you can use it without paying a licensing fee.
 */

#include <compiler.h>
#include <sysclk.h>
#include <board.h>
#include <gpio.h>
#include <os_core.h>

#include "conf_example.h"

#define MAX_DELAY_MS 500

static struct os_mutex mutex;

struct task_args {
	uint32_t pin;
	os_tick_t tick_nb;
};

void led_blink(os_ptr_t raw_args)
{
	struct task_args *args = (struct task_args *) raw_args;
	while (true) {
		os_mutex_lock(&mutex);
		gpio_tgl_gpio_pin(args->pin);
		os_task_delay(args->tick_nb);
		os_mutex_unlock(&mutex);
	}
}

int main(void)
{
	struct os_task tasks[EXAMPLE_NB_TASKS];
	struct task_args args[EXAMPLE_NB_TASKS];
	int i;

	sysclk_init();
	board_init();

	os_mutex_create(&mutex);

	for (i=0; i<EXAMPLE_NB_TASKS; i++) {
		args[i].pin = example_pins[i];
		args[i].tick_nb = OS_MS_TO_TICKS(MAX_DELAY_MS);
		os_task_create(&tasks[i], led_blink, &args[i], 200,
				OS_TASK_DEFAULT);
	}

	os_start(sysclk_get_cpu_hz());
}
