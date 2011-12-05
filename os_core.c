#include "compiler.h"
#include "os_core.h"

static struct os_task_minimal os_app = {
	.next = (struct os_task *) &os_app,
#if CONFIG_OS_USE_PRIORITY == true
	.priority = OS_TASK_PRIORITY_1,
	.priority_counter = OS_TASK_PRIORITY_1,
#endif
};
struct os_task *os_current_task = (struct os_task *) &os_app;
#if CONFIG_OS_USE_TICK_COUNTER == true
volatile os_tick_t tick_counter = 0;
#endif

#if CONFIG_OS_USE_PRIORITY == true
void os_task_scheduler_priority(void);
void os_task_scheduler_priority(void)
{
	do {
		// Get the next task
		os_current_task = os_current_task->core.next;
		// Check wether its priority counter is null
		if (!os_current_task->core.priority_counter) {
			os_current_task->core.priority_counter = os_current_task->core.priority;
			return;
		}
		// Decrease the priority counter
		os_current_task->core.priority_counter--;
	} while (true);
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
		while (tick_counter > start_tick);
		while (tick_counter < last_tick);
	}
	else {
		while (tick_counter < last_tick);
	}
}
#endif

bool os_task_add(struct os_task *task, task_ptr_t task_ptr, void *args,
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
	// Move the SP pointer to the end of the stack
	task->core.sp = &task->stack[stack_size];
	// Save the options
	task->options = options;
	// Load context
	if (!os_task_context_load(task, task_ptr, args)) {
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

struct os_task *os_task_current(void) {
	if (os_current_task == (struct os_task *) &os_app) {
		return NULL;
	}
	return os_current_task;
}

/*!
 * All the actives tasks are stored in a chain list.
 * The current task is the task pointed by \ref os_current_task.
 * At the begining, when the task scheduler is not running the current process
 * is not task so its infromation need to be storted in a specific context.
 * This context is \ref os_app. Therefore, at the begining the active task chain
 * list looks like this:
 * os_current_task -> os_app -> task1 -> task2 -> task1 -> task2 -> ...
 * During normal execution, os_app is not part of the cahin list anymore:
 * os_current_task -> task1 -> task2 -> task1 -> task2 -> ...
 */
void os_task_enable(struct os_task *task)
{
	struct os_task *last_task = task;

	// The following code is critical
	os_enter_critical();

	// Make sure the task is not already enabled
	if (!os_task_is_enabled(task)) {
		// Look for the last task registered
		last_task = os_current_task->core.next;
		while (last_task->core.next != os_current_task->core.next) {
			last_task = last_task->core.next;
		}
		// Add the task to the chain list
		if (last_task == (struct os_task *) &os_app) {
			task->core.next = task;
		}
		else {
			task->core.next = last_task->core.next;
		}
		last_task->core.next = task;
	}

	os_leave_critical();
}

bool os_task_is_enabled(struct os_task *task)
{
	// Starts from the "next" element.
	// There is maximum 1 element on the path before reaching the circular
	// chain buffer.
	struct os_task *last_task = os_current_task->core.next;
	do {
		if (last_task == task) {
			return true;
		}
		last_task = last_task->core.next;
	} while (last_task != os_current_task->core.next);
	return false;
}

void os_task_disable(struct os_task *task)
{
	struct os_task *last_task = task;

	// Unregister this task from the active task list
	os_enter_critical();

	// Make sure the task is already enabled
	if (os_task_is_enabled(task)) {
		// Look for the last task
		while (last_task->core.next != task) {
			last_task = last_task->core.next;
		}
		// Remove the task from the chain list
		if (last_task == task) {
			os_app.next = (struct os_task *) &os_app;
			task->core.next = (struct os_task *) &os_app;
		}
		else {
			last_task->core.next = task->core.next;
		}
	}

	os_task_yield();
	os_leave_critical();
}

void os_task_yield(void)
{
	os_enter_critical();
	os_task_switch_context();
	os_leave_critical();
}
