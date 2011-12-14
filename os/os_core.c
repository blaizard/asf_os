/*! \file
 * \brief eeOS Core
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

#include "compiler.h"
#include "os_core.h"

/*! Internal hooks
 * \{
 */
#ifndef OS_SCHEDULER_PRE_INTERRUPT_HOOK
	#define OS_SCHEDULER_PRE_INTERRUPT_HOOK()
#endif
/*!
 * \}
 */

/*! \brief Context of the application task.
 * \note This context is available only after the call of \ref os_start
 */
struct os_task_minimal os_app = {
	.next = &os_app,
#if CONFIG_OS_USE_PRIORITY == true
	.priority = OS_PRIORITY_1,
	.priority_counter = OS_PRIORITY_1,
#endif
};

/*! \brief Current task running
 */
struct os_task_minimal *os_current_task = &os_app;

#if CONFIG_OS_USE_TICK_COUNTER == true
/*! \brief Tick counter
 */
volatile os_tick_t tick_counter = 0;
#endif

#if CONFIG_OS_USE_PRIORITY == true
struct os_task_minimal *os_task_scheduler(void)
{
	do {
		// Get the next task
		os_current_task = os_current_task->next;
		// Check wether its priority counter is null
		if (!os_current_task->priority_counter) {
			os_current_task->priority_counter = os_current_task->priority;
			OS_SCHEDULER_PRE_INTERRUPT_HOOK();
			return os_current_task;
		}
		// Decrease the priority counter
		os_current_task->priority_counter--;
	} while (true);
}
#else
struct os_task_minimal *os_task_scheduler(void)
{
	os_current_task = os_current_task->next;
	OS_SCHEDULER_PRE_INTERRUPT_HOOK();
	return os_current_task;
}
#endif

#if CONFIG_OS_USE_TICK_COUNTER == true
void os_task_delay(os_tick_t tick_nb)
{
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

bool os_task_create(struct os_task *task, os_task_ptr_t task_ptr, os_ptr_t args,
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
	task->core.sp = &task->stack[stack_size];
#if CONFIG_OS_USE_PRIORITY == true
	os_task_set_priority(task, CONFIG_OS_TASK_DEFAULT_PRIORITY);
#endif
	// Load context
	if (!os_task_context_load(&task->core, task_ptr, args)) {
		return false;
	}
	// Enable the task
	task->core.next = NULL;
	if (!(options & OS_TASK_DISABLE)) {
		os_task_enable(task);
	}

	return true;
}

void os_task_delete(struct os_task *task)
{
	// Remove the task from the active task list
	os_task_disable(task);
	if (!(task->options & OS_TASK_USE_CUSTOM_STACK)) {
		// Free the stack
		os_free(task->stack);
	}
}

struct os_task *os_task_current(void)
{
	if (os_current_task == &os_app) {
		return NULL;
	}
	return (struct os_task *) os_current_task;
}

void __os_task_enable(struct os_task_minimal *task)
{
	struct os_task_minimal *last_task = task;

	// Look for the last task registered
	last_task = os_current_task->next;
	while (last_task->next != os_current_task->next) {
		last_task = last_task->next;
	}
	// Add the task to the chain list. Different behavior regarding the
	// application task because for the event handler uses this task to run
#if CONFIG_OS_USE_EVENTS == true
	// The application task is disabled in the event scheduler
	task->next = last_task->next;
#else
	// If the application task is running, remove it from the active task
	// list
	if (last_task == &os_app) {
		task->next = task;
	}
	else {
		task->next = last_task->next;
	}
#endif
	last_task->next = task;
}

void os_task_enable(struct os_task *task)
{
	// The following code is critical
	os_enter_critical();
	// Make sure the task is not already enabled
	if (!os_task_is_enabled(task)) {
		__os_task_enable(&task->core);
	}
	os_leave_critical();
}

bool os_task_is_enabled(struct os_task *task)
{
	// Starts from the "next" element.
	// There is maximum 1 element on the path before reaching the circular
	// chain buffer.
	struct os_task_minimal *last_task = os_current_task->next;
	do {
		if (last_task == (struct os_task_minimal *) task) {
			return true;
		}
		last_task = last_task->next;
	} while (last_task != os_current_task->next);
	return false;
}

void __os_task_disable(struct os_task_minimal *task)
{
	struct os_task_minimal *last_task = task;

	// Look for the last task
	while (last_task->next != task) {
		last_task = last_task->next;
	}
	// Remove the task from the chain list
	if (last_task == task) {
		os_app.next = &os_app;
		task->next = &os_app;
	}
	else {
		last_task->next = task->next;
	}
}

void os_task_disable(struct os_task *task)
{
	// Unregister this task from the active task list
	os_enter_critical();
	// Make sure the task is enabled
	if (os_task_is_enabled(task)) {
		__os_task_disable(&task->core);
	}
	os_task_switch_context(false);
	os_leave_critical();
}

void os_yield(void)
{
	os_enter_critical();
	os_task_switch_context(false);
	os_leave_critical();
}
