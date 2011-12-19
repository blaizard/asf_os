/*! \file
 * \brief eeOS Mutex
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

/*! \name Private functions
 * \{
 */
static enum os_event_status __os_event_mutex_is_triggered(struct os_process *proc,
		os_ptr_t args);
/*!
 * \}
 */

const struct os_event_descriptor mutex_event_descriptor = {
	.is_triggered = __os_event_mutex_is_triggered
};

static enum os_event_status __os_event_mutex_is_triggered(struct os_process *proc,
		os_ptr_t args)
{
	enum os_event_status status = OS_EVENT_NONE;
	struct os_mutex *mutex = (struct os_mutex *) args;
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	if (!mutex->is_locked) {
		mutex->is_locked = true;
		mutex->process = proc;
		status = OS_EVENT_OK_STOP;
	}
	if (!is_critical) {
		os_leave_critical();
	}

	return status;
}

void os_mutex_lock(struct os_mutex *mutex)
{
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	/* If the mutex is not locked, lock it */
	if (!mutex->is_locked) {
		mutex->is_locked = true;
		mutex->process = os_process_get_current();
	}
	/* If the mutex is already locked, suspend this task */
	else {
		/* Disable this process */
		__os_process_disable(os_process_get_current());
		/* Add this process to the event list of the mutex */
		os_waiting_list_add(&mutex->next,
				os_process_get_current());
		/* Manually switch the process context */
		os_switch_context(false);
	}
	/* Leave the critical region unless the CPU was previously in */
	if (!is_critical) {
		os_leave_critical();
	}
}

void os_mutex_unlock(struct os_mutex *mutex)
{
	/* Only the process which locked the mutex can unlock it */
	if (os_process_get_current() == mutex->process) {
		/* Save the critical region status */
		bool is_critical = os_is_critical();
		/* Enter in a critical region if not already in */
		if (!is_critical) {
			os_enter_critical();
		}
		/* Check if there is another process in the waiting list */
		if (mutex->next) {
			struct os_process *proc;
			/* Pop the next process in the waiting list */
			proc = os_waiting_list_pop(&mutex->next);
			/* Lock the mutex for this process */
			mutex->process = proc;
			/* Enable this process */
			__os_process_enable(proc);
		}
		/* Else unlock the mutex */
		else {
			mutex->is_locked = false;
		}
		/* Leave the critical region unless the CPU was previously in */
		if (!is_critical) {
			os_leave_critical();
		}
	}
}
