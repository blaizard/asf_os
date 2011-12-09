#include "os_core.h"

static struct os_event *os_current_event = NULL;

static inline struct os_task_minimal *os_event_pop_task(struct os_event *event) {
	struct os_task_minimal *task = event->task;
	event->task = task->next;
	return task;
}

/*! \note event MUST be in the event list
 */
static inline void os_event_pop(struct os_event *event) {
	struct os_event *prev_event = os_current_event;
	// If the event is the 1rst one
	if (event == os_current_event) {
		os_current_event = event->next;
	}
	else {
		// Look for the event in the list
		while (prev_event->next != event) {
			prev_event = prev_event->next;
		}
		prev_event->next = event->next;
	}
}

/*!
 * Events
 */
bool os_event_scheduler(void)
{
	// Current event to process
	struct os_event *event = os_current_event;
	struct os_task_minimal *task;
	enum os_event_status status;

	// If no event, return
	if (!event) {
		return false;
	}

	// Loop through the event list
	do {
		do {
			// Check if the event has been triggered
			status = event->desc.is_triggered((void *) event->args);
			// >= to make sure the compiler will optimize it
			if (status >= OS_EVENT_OK_STOP) {
				os_enter_critical();
				// Remove the task from the event list
				task = os_event_pop_task(event);
				// If this is the last task, remove the event
				// from the list
				if (!task->next) {
					status = OS_EVENT_OK_STOP;
					os_event_pop(event);
				}
				// Activate the task/interrupt
				__os_task_enable(task);
				os_leave_critical();
			}
		} while (status == OS_EVENT_OK_CONTINUE);
		// Next event
		event = event->next;
	} while (event);

	// There is at least 1 event in the queue
	return true;
}
