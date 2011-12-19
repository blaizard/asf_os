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

static inline struct os_process *__os_event_pop_process(struct os_event *event) {
	struct os_process *proc = event->proc;
	event->proc = proc->next;
	return proc;
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

static inline void __os_event_insert_process_after(struct os_process *proc,
		struct os_process *new_proc) {
	new_proc->next = proc->next;
	proc->next = new_proc;
}

static inline void __os_event_insert_process_begining(struct os_event *event,
		struct os_process *proc) {
	proc->next = event->proc;
	event->proc = proc;
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

bool os_event_sort_fifo(struct os_process *proc1,
		struct os_process *proc2)
{
	return true;
}

bool os_event_sort_lifo(struct os_process *proc1,
		struct os_process *proc2)
{
	return false;
}

#if CONFIG_OS_USE_PRIORITY == true
bool os_event_sort_priority(struct os_process *proc1,
		struct os_process *proc2)
{
	return (proc1->priority <= proc2->priority);
}
#endif

void __os_event_register(struct os_event *event, struct os_process *proc)
{
	struct os_event *current_event;
	struct os_process *prev_proc = NULL;
	struct os_process *current_proc;
	bool (*sort_fct)(struct os_process *, struct os_process *);

	// Get the appropriate sorting function
#if CONFIG_OS_USE_PRIORITY == true
	sort_fct = os_event_sort_priority;
#else
	sort_fct = os_event_sort_fifo;
#endif
	if (event->desc.sort) {
		sort_fct = event->desc.sort;
	}

	// Enable the event process if not done already, before messing up
	// with the process context
	__os_process_event_enable();

	// Add the process to the event sorted process list
	current_proc = event->proc;
	while (current_proc && sort_fct(current_proc, proc)) {
		prev_proc = current_proc;
		current_proc = current_proc->next;
	}
	// If the process is supposed to be at the beginning of the list
	if (prev_proc) {
		__os_event_insert_process_after(prev_proc, proc);
	}
	else {
		__os_event_insert_process_begining(event, proc);
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
void os_event_scheduler(void)
{
	// Current event to process
	struct os_event *event = os_current_event;
	struct os_process *proc;
	enum os_event_status status;

	// If no events, disable the event process
	if (!event) {
		__os_process_event_disable();
	}

	// Loop through the event list
	os_enter_critical();
	while (event) {
		do {
			// Check if the event has been triggered
			status = event->desc.is_triggered(event->proc,
					event->args);
			if (status != OS_EVENT_NONE) {
				// Remove the process from the event list
				proc = __os_event_pop_process(event);
				// If this is the last process, remove the event
				// from the list
				if (!proc->next) {
					status = OS_EVENT_OK_STOP;
					__os_event_pop(event);
				}
				// Activate the process
				__os_process_enable(proc);
			}
		} while (status == OS_EVENT_OK_CONTINUE);
		// Next event
		event = event->next;
	}
	os_leave_critical();

	// Call the scheduler
	os_yield();
}

struct __os_event_custom_function_args {
	bool (*trigger)(os_ptr_t);
	os_ptr_t args;
};

enum os_event_status __os_event_custom_function_handler(struct os_process *proc,
		os_ptr_t args);
enum os_event_status __os_event_custom_function_handler(struct os_process *proc,
		os_ptr_t args)
{
	struct __os_event_custom_function_args *custom_args =
			(struct __os_event_custom_function_args *) args;
	if (custom_args->trigger(custom_args->args))
		return OS_EVENT_OK_CONTINUE;
	return OS_EVENT_NONE;
}

void os_event_create_from_function(struct os_event *event,
		bool (*trigger)(os_ptr_t), os_ptr_t args)
{
	const struct os_event_descriptor descriptor = {
		.is_triggered = __os_event_custom_function_handler
	};
	struct __os_event_custom_function_args custom_args = {
		.trigger = trigger,
		.args = args
	};
	os_event_create(event, &descriptor, (os_ptr_t) &custom_args);
}

/*!
 * Task API Extension
 * \{
 */
struct os_process __os_event_alternate_proc = {
	.next = NULL
};

#if CONFIG_OS_USE_SW_INTERRUPTS == true
void os_interrupt_trigger_on_event(struct os_interrupt *interrupt,
		struct os_event *event)
{
	/* Save the critical region status */
	bool is_critical = os_is_critical();
	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	__os_event_start(event);
	__os_event_register(event, os_interrupt_get_process(interrupt));
	if (!is_critical) {
		os_leave_critical();
	}
}
#endif

void os_task_sleep(struct os_task *task, struct os_event *event)
{
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Start the event */
	__os_event_start(event);

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}
	/* Send the task to sleep if not done already */
	if (os_task_is_enabled(task)) {
		__os_process_disable(os_task_get_process(task));
	}
	/* Save the next current task pointer because it will be erased
	 * by the sleep operation if the current process is the process
	 * to go to sleep.
	 */
	__os_event_alternate_proc.next = os_process_get_current()->next;
	/* Associate the task with its event and start it */
	__os_event_register(event, os_task_get_process(task));
	/* Call the scheduler */
	os_switch_context(false);
	/* Leave the critical region */
	if (!is_critical) {
		os_leave_critical();
	}
}
/*!
 * \}
 */

#endif // CONFIG_OS_USE_EVENTS == true
