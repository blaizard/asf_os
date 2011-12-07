#include "os_core.h"

/*
static struct os_task_minimal os_event_task = {
	
};
*/
static struct os_event *os_current_event = NULL;

static inline struct os_task_minimal *os_event_pop_task(struct os_event *event) {
	struct os_task_minimal *task = event->task;
	event->task = task->next;
	return task;
}

void os_event_load(void)
{
	
}

void os_event_scheduler(void)
{
	// Current event to process
	struct os_event *event = os_current_event;
	struct os_task_minimal *task;
	enum os_event_status status;

	os_enter_critical();
	// Loop through the event list
	while (event) {
		do {
			// Check if the event has been triggered
			status = event->is_triggered((void *) event);
			// >= to make sure the compiler will optimize it
			if (status >= OS_EVENT_OK_STOP) {
				task = os_event_pop_task(event);
				// Enable the task
				__os_task_enable(task);
			}
		} while (status == OS_EVENT_OK_CONTINUE);
		// Next event
		event = event->next;
	}
	os_leave_critical();
}
