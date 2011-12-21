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
	.status = OS_PROCESS_ACTIVE,
	.type = OS_PROCESS_TYPE_APPLICATION,
#if CONFIG_OS_USE_PRIORITY == true
	.priority = OS_PRIORITY_1,
	.priority_counter = OS_PRIORITY_1,
#endif
#if CONFIG_OS_STATISTICS_MONITOR_TASK_SWITCH == true
	.cycle_counter = 0,
#endif
};

/*! \brief Current process running
 */
struct os_process *__os_current_process = &os_app;

#if CONFIG_OS_USE_TICK_COUNTER == true
/*! \brief Tick counter
 */
volatile os_tick_t os_tick_counter = 0;
#endif

#if CONFIG_OS_USE_PRIORITY == true
struct os_process *os_scheduler(void)
{
	do {
		// Get the next process
		__os_current_process = __os_current_process->next;
		// Check wether its priority counter is null
		if (__os_current_process->priority_counter == 0) {
			__os_current_process->priority_counter =
					__os_current_process->priority;
			OS_SCHEDULER_PRE_INTERRUPT_HOOK();
			OS_DEBUG_TRACE_LOG(OS_DEBUG_TRACE_CONTEXT_SWITCH, __os_current_process);
			return __os_current_process;
		}
		// Decrease the priority counter
		__os_current_process->priority_counter--;
	} while (true);
}
#else
struct os_process *os_scheduler(void)
{
	__os_current_process = __os_current_process->next;
	OS_SCHEDULER_PRE_INTERRUPT_HOOK();
	OS_DEBUG_TRACE_LOG(OS_DEBUG_TRACE_CONTEXT_SWITCH, __os_current_process);
	return __os_current_process;
}
#endif

void __os_process_enable(struct os_process *proc)
{
	struct os_process *last_proc = proc;

	// Look for the last process registered
	last_proc = __os_current_process->next;
#if CONFIG_OS_PROCESS_ENABLE_FIFO == true
	while (last_proc->next != __os_current_process->next) {
		last_proc = last_proc->next;
	}
#endif
	// Add the process to the chain list.
	// If the application process is running, remove it from the active process
	// list
	if (os_process_is_application(last_proc)) {
		proc->next = proc;
		os_app.status = OS_PROCESS_IDLE;
	}
	else {
		proc->next = last_proc->next;
	}
	proc->status = OS_PROCESS_ACTIVE;
	last_proc->next = proc;
}

void os_process_enable(struct os_process *proc)
{
	bool is_critical = os_is_critical();
	// The following code is critical
	if (!is_critical) {
		os_enter_critical();
	}
	// Make sure the task is not already enabled
	if (!os_process_is_enabled(proc)) {
		__os_process_enable(proc);
	}
	if (!is_critical) {
		os_leave_critical();
	}
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
		os_app.status = OS_PROCESS_ACTIVE;
		proc->next = &os_app;
		os_app.type = OS_PROCESS_TYPE_APPLICATION;
	}
	else {
		last_proc->next = proc->next;
	}
	proc->status = OS_PROCESS_IDLE;
}

void os_process_disable(struct os_process *proc)
{
	bool is_critical = os_is_critical();
	// The following code is critical
	if (!is_critical) {
		os_enter_critical();
	}
	// Make sure the process is enabled
	if (os_process_is_enabled(proc)) {
		__os_process_disable(proc);
	}
	os_switch_context(false);
	if (!is_critical) {
		os_leave_critical();
	}
}

void os_yield(void)
{
	bool is_critical = os_is_critical();
	// The following code is critical
	if (!is_critical) {
		os_enter_critical();
	}
	OS_DEBUG_TRACE_LOG(OS_DEBUG_TRACE_YIELD, __os_current_process);
	os_switch_context(false);
	if (!is_critical) {
		os_leave_critical();
	}
}
