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
struct os_process os_app = {
	.next = &os_app,
#if CONFIG_OS_USE_PRIORITY == true
	.priority = OS_PRIORITY_1,
	.priority_counter = OS_PRIORITY_1,
#endif
};

/*! \brief Current process running
 */
struct os_process *os_current_process = &os_app;

#if CONFIG_OS_USE_TICK_COUNTER == true
/*! \brief Tick counter
 */
volatile os_tick_t tick_counter = 0;
#endif

#if CONFIG_OS_USE_PRIORITY == true
struct os_process *os_scheduler(void)
{
	do {
		// Get the next process
		os_current_process = os_current_process->next;
		// Check wether its priority counter is null
		if (!os_current_process->priority_counter) {
			os_current_process->priority_counter =
					os_current_process->priority;
			OS_SCHEDULER_PRE_INTERRUPT_HOOK();
			return os_current_process;
		}
		// Decrease the priority counter
		os_current_process->priority_counter--;
	} while (true);
}
#else
struct os_process *os_scheduler(void)
{
	os_current_process = os_current_process->next;
	OS_SCHEDULER_PRE_INTERRUPT_HOOK();
	return os_current_process;
}
#endif

struct os_process *os_get_current_process(void)
{
	return os_current_process;
}

void __os_process_enable(struct os_process *proc)
{
	struct os_process *last_proc = proc;

	// Look for the last process registered
	last_proc = os_current_process->next;
	while (last_proc->next != os_current_process->next) {
		last_proc = last_proc->next;
	}
	// Add the process to the chain list. Different behavior regarding the
	// application task because for the event handler uses this proc to run
#if CONFIG_OS_USE_EVENTS == true
	// The application process is disabled in the event scheduler
	proc->next = last_proc->next;
#else
	// If the application process is running, remove it from the active process
	// list
	if (last_proc == &os_app) {
		proc->next = proc;
	}
	else {
		proc->next = last_proc->next;
	}
#endif
	last_proc->next = proc;
}

void os_process_enable(struct os_process *proc)
{
	// The following code is critical
	os_enter_critical();
	// Make sure the task is not already enabled
	if (!os_process_is_enabled(proc)) {
		__os_process_enable(proc);
	}
	os_leave_critical();
}

bool os_process_is_enabled(struct os_process *proc)
{
	// Starts from the "next" element.
	// There is maximum 1 element on the path before reaching the circular
	// chain buffer.
	struct os_process *last_proc = os_current_process->next;
	do {
		if (last_proc == proc) {
			return true;
		}
		last_proc = last_proc->next;
	} while (last_proc != os_current_process->next);
	return false;
}

void __os_process_disable(struct os_process *proc)
{
	struct os_process *last_proc = proc;

	// Look for the last process
	while (last_proc->next != proc) {
		last_proc = last_proc->next;
	}
	// Remove the process from the chain list
	if (last_proc == proc) {
		// If this was the last process in the chain list, then remove
		// it and add the application process instead
		os_app.next = &os_app;
		proc->next = &os_app;
	}
	else {
		last_proc->next = proc->next;
	}
}

void os_process_disable(struct os_process *proc)
{
	// Unregister this process from the active process list
	os_enter_critical();
	// Make sure the process is enabled
	if (os_process_is_enabled(proc)) {
		__os_process_disable(proc);
	}
	os_switch_context(false);
	os_leave_critical();
}

void os_yield(void)
{
	os_enter_critical();
	os_switch_context(false);
	os_leave_critical();
}
