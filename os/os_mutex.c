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

	os_enter_critical();
	if (!mutex->is_locked) {
		mutex->is_locked = true;
		mutex->process = proc;
		status = OS_EVENT_OK_STOP;
	}
	os_leave_critical();

	return status;
}

void os_mutex_lock(struct os_mutex *mutex)
{
	bool is_taken = false;

	do {
		os_enter_critical();
		if (!mutex->is_locked) {
			mutex->is_locked = true;
			mutex->process = os_process_get_current();
			is_taken = true;
		}
		os_leave_critical();

		if (!is_taken) {
			os_yield();
		}

	} while (!is_taken);
}

void os_mutex_unlock(struct os_mutex *mutex)
{
	// Only the process which locked the mutex can unlock it
	if (os_process_get_current() == mutex->process) {
		os_enter_critical();
		mutex->is_locked = false;
		os_leave_critical();
	}
}
