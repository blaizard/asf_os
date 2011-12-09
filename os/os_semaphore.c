#include "os_core.h"

/*! \name Private functions
 * \{
 */
static enum os_event_status os_event_sempahore_is_triggered(void *args);
#if CONFIG_OS_USE_PRIORITY == true
static bool os_event_semaphore_priority_sort(struct os_task_minimal *task1,
		struct os_task_minimal *task2)
{
	return (task1->priority <= task2->priority);
}
#endif
/*!
 * \}
 */

const struct os_event_descriptor semaphore_event_descriptor = {
#if CONFIG_OS_USE_PRIORITY == true
	.sort = os_event_semaphore_priority_sort,
#endif
	.is_triggered = os_event_sempahore_is_triggered
};

/*!
 * \brief Take the semphore, use by the event.
 * \param event The semaphore event
 * \return OS_EVENT_NONE if no ressource is available. OS_EVENT_OK_STOP if
 * only 1 is available, OS_EVENT_OK_CONTINUE if more than one.
 */
static enum os_event_status os_event_sempahore_is_triggered(void *args)
{
	enum os_event_status status = OS_EVENT_NONE;
	struct os_semaphore *sem = (struct os_semaphore *) args;

	os_enter_critical();
	if (sem->counter == 1) {
		sem->counter = 0;
		status = OS_EVENT_OK_STOP;
	}
	else if (sem->counter > 1) {
		sem->counter--;
		status = OS_EVENT_OK_CONTINUE;
	}
	os_leave_critical();

	return status;
}

void os_semaphore_take(struct os_semaphore *sem)
{
	bool is_taken = false;

	do {
		os_enter_critical();
		if (sem->counter > 0) {
			sem->counter--;
			is_taken = true;
		}
		os_leave_critical();
	} while (!is_taken);
}

void os_sempahore_release(struct os_semaphore *sem)
{
	os_enter_critical();
	if (sem->counter < sem->max) {
		sem->counter++;
	}
	os_leave_critical();
}

