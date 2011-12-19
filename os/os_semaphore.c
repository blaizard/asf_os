/*! \file
 * \brief eeOS Semaphores
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
static enum os_event_status os_event_sempahore_is_triggered(struct os_process *proc,
		os_ptr_t args);
/*!
 * \}
 */

const struct os_event_descriptor semaphore_event_descriptor = {
	.is_triggered = os_event_sempahore_is_triggered
};

/*!
 * \brief Take the semphore, use by the event.
 * \param event The semaphore event
 * \return OS_EVENT_NONE if no ressource is available. OS_EVENT_OK_STOP if
 * only 1 is available, OS_EVENT_OK_CONTINUE if more than one.
 */
static enum os_event_status os_event_sempahore_is_triggered(struct os_process *proc,
		os_ptr_t args)
{
	enum os_event_status status = OS_EVENT_NONE;
	struct os_semaphore *sem = (struct os_semaphore *) args;
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	if (sem->counter == 1) {
		sem->counter = 0;
		status = OS_EVENT_OK_STOP;
	}
	else if (sem->counter > 1) {
		sem->counter--;
		status = OS_EVENT_OK_CONTINUE;
	}
	if (!is_critical) {
		os_leave_critical();
	}

	return status;
}

void os_semaphore_take(struct os_semaphore *sem)
{
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	/* If all the semaphores are not taken, take one */
	if (sem->counter > 0) {
		/* Decrease the semaphore counter */
		sem->counter--;
	}
	/* If the all the semaphores are taken, suspend this task */
	else {
		/* Disable this process */
		__os_process_disable(os_process_get_current());
		/* Add this process to the event list of the sempahore */
		os_waiting_list_add(&sem->next,
				os_process_get_current());
		/* Manually switch the process context */
		os_switch_context(false);
	}
	/* Leave the critical region unless the CPU was previously in */
	if (!is_critical) {
		os_leave_critical();
	}
}

void os_semaphore_release(struct os_semaphore *sem)
{
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	/* Check if there is another process in the waiting list */
	if (sem->next) {
		struct os_process *proc;
		/* Pop the next process in the waiting list */
		proc = os_waiting_list_pop(&sem->next);
		/* Enable this process */
		__os_process_enable(proc);
	}
	/* Else check if the sempahore counter is not above the limit */
	else if (sem->counter < sem->max) {
		/* Increase the semaphore counter, in other word, release the
		 * semaphore previously taken.
		 */
		sem->counter++;
	}
	/* Leave the critical region unless the CPU was previously in */
	if (!is_critical) {
		os_leave_critical();
	}
}

