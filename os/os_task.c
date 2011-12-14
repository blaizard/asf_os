/*! \file
 * \brief eeOS Tasks
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

#include "os_core.h"

#if CONFIG_OS_USE_TICK_COUNTER == true
void os_task_delay(os_tick_t tick_nb)
{
	extern volatile os_tick_t tick_counter;

	os_tick_t start_tick, last_tick;
	start_tick = tick_counter;
	last_tick = tick_counter + tick_nb;
	// Check if the counter has been wrapped
	if (last_tick < start_tick) {
		while (tick_counter > start_tick) {
			os_yield();
		}
	}
	while (tick_counter < last_tick) {
		os_yield();
	}
}
#endif

bool os_task_create(struct os_task *task, os_proc_ptr_t task_ptr, os_ptr_t args,
		int stack_size, enum os_task_option options)
{
#if CONFIG_OS_USE_MALLOC == true
	if (!(options & OS_TASK_USE_CUSTOM_STACK)) {
		// Allocate memory for the stack size
		if (!(task->stack = os_malloc(stack_size))) {
			return false;
		}
	}
#endif
#if CONFIG_OS_DEBUG == true
	HOOK_OS_DEBUG_TASK_ADD();
#endif
	// Save the options
	task->options = options;
	// Move the SP pointer to the end of the stack
	os_task_get_process(task)->sp = &task->stack[stack_size];
	// Set process type
	os_task_get_process(task)->type = OS_PROCESS_TYPE_TASK;
#if CONFIG_OS_USE_PRIORITY == true
	os_task_set_priority(task, CONFIG_OS_TASK_DEFAULT_PRIORITY);
#endif
	// Load context
	if (!os_process_context_load(&task->core, task_ptr, args)) {
		return false;
	}
	// Enable the task
	if (!(options & OS_TASK_DISABLE)) {
		os_task_enable(task);
	}

	return true;
}

struct os_task *os_task_get_current(void)
{
	extern struct os_process *os_current_process;
	extern struct os_process os_app;
	if (os_current_process == &os_app) {
		return NULL;
	}
	return os_task_from_process(os_current_process);
}
