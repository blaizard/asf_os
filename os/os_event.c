/*! \file
 * \brief eeOS Events
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

#if CONFIG_OS_USE_EVENTS == true

static struct os_event *os_current_event = NULL;

void os_event_create(struct os_event *event,
		const struct os_event_descriptor *descriptor, os_ptr_t args)
{
	// Fill the event structure
	event->desc.sort = descriptor->sort;
	event->desc.start = descriptor->start;
	event->desc.is_triggered = descriptor->is_triggered;
	event->args = args;
}

/*! Private helper functions
 * \{
 */

static inline struct os_task_minimal *__os_event_pop_task(struct os_event *event) {
	struct os_task_minimal *task = event->task;
	event->task = task->next;
	return task;
}

/*! \note event MUST be in the event list
 */
static inline void __os_event_pop(struct os_event *event) {
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

static inline void __os_event_insert_task_after(struct os_task_minimal *task,
		struct os_task_minimal *new_task) {
	new_task->next = task->next;
	task->next = new_task;
}

static inline void __os_event_insert_task_begining(struct os_event *event,
		struct os_task_minimal *task) {
	task->next = event->task;
	event->task = task;
}

static inline void __os_event_enable(struct os_event *event) {
	if (os_current_event) {
		event->next = os_current_event->next;
	}
	else {
		event->next = NULL;
	}
	os_current_event = event;
}

static inline void __os_event_start(struct os_event *event) {
	if (event->desc.start) {
		event->desc.start(event->args);
	}
}

/*!
 * \}
 */

bool os_event_sort_fifo(struct os_task_minimal *task1,
		struct os_task_minimal *task2)
{
	return true;
}

bool os_event_sort_lifo(struct os_task_minimal *task1,
		struct os_task_minimal *task2)
{
	return false;
}

void __os_event_register(struct os_event *event, struct os_task_minimal *task)
{
	struct os_event *current_event;
	struct os_task_minimal *prev_task = NULL;
	struct os_task_minimal *current_task;
	bool (*sort_fct)(struct os_task_minimal *, struct os_task_minimal *);

	// Get the appropriate sorting function
	sort_fct = os_event_sort_fifo;
	if (event->desc.sort) {
		sort_fct = event->desc.sort;
	}

	// Enable the application task if not done already, before messing up
	// with the task context
	__os_task_enable_application();

	// Add the task to the event sorted task list
	current_task = event->task;
	while (current_task && sort_fct(current_task, task)) {
		prev_task = current_task;
		current_task = current_task->next;
	}
	// If the task is supposed to be at the beginning of the list
	if (prev_task) {
		__os_event_insert_task_after(prev_task, task);
	}
	else {
		__os_event_insert_task_begining(event, task);
	}

	// Add this event to the event list if not done already
	current_event = os_current_event;
	while (current_event != event) {
		if (current_event == NULL) {
			__os_event_enable(event);
			break;
		}
		current_event = current_event->next;
	}
}

/*!
 * Event scheduler
 */
bool os_event_scheduler(void)
{
	// Current event to process
	struct os_event *event = os_current_event;
	struct os_task_minimal *task;
	enum os_event_status status;

	// If no event, return
	if (!event) {
		// Disable the application task
		__os_task_disable_application();
		return false;
	}

	// Loop through the event list
	do {
		do {
			// Check if the event has been triggered
			status = event->desc.is_triggered((os_ptr_t) event->args);
			// >= to make sure the compiler will optimize it
			if (status >= OS_EVENT_OK_STOP) {
				os_enter_critical();
				// Remove the task from the event list
				task = __os_event_pop_task(event);
				// If this is the last task, remove the event
				// from the list
				if (!task->next) {
					status = OS_EVENT_OK_STOP;
					__os_event_pop(event);
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

void os_event_create_from_function(struct os_event *event, bool (*trigger)(os_ptr_t))
{
}

/*!
 * Task API Extension
 * \{
 */
struct os_task_minimal os_event_alternate_task = {
	.next = NULL
};

#if CONFIG_OS_USE_SW_INTERRUPTS == true
void os_interrupt_sleep(struct os_interrupt *interrupt, struct os_event *event)
{
	os_enter_critical();
	__os_event_start(event);
	__os_event_register(event, (struct os_task_minimal *) interrupt);
	os_leave_critical();
}
#endif

void os_task_sleep(struct os_task *task, struct os_event *event)
{
	extern struct os_task_minimal *os_current_task;

	// Start the event
	__os_event_start(event);

	os_enter_critical();
	// If the task is enabled, send it to sleep
	if (os_task_is_enabled(task)) {
		__os_task_disable(&task->core);
	}
	// Save the next current task pointer because it will be erase by the
	// sleep operation
	os_event_alternate_task.next = os_current_task->next;
	// Associate the task with its event and start it
	__os_event_register(event, (struct os_task_minimal *) task);
	// Call the scheduler
	os_task_switch_context(false);
	os_leave_critical();
}
/*!
 * \}
 */

#endif // CONFIG_OS_USE_EVENTS == true
