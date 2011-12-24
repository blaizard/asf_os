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
	/* Mark this event as disabled */
	event->queue.next = NULL;
	event->queue.prev = NULL;
}

/*! Private helper functions
 * \{
 */

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

static inline void __os_event_enable(struct os_event *event) {
	if (os_current_event) {
		event->next = os_current_event;
	}
	else {
		event->next = NULL;
	}
	os_current_event = event;
}

static inline void __os_event_start(struct os_event *event,
		struct os_process *proc) {
	if (event->desc.start) {
		event->desc.start(proc, event->args);
	}
}

/*!
 * \}
 */

/*!
 * Event scheduler
 */
void os_event_scheduler(void)
{
	// Current event to process
	struct os_event *event;
	enum os_event_status status = OS_EVENT_NONE;

	// Loop through the event list
	os_enter_critical();
	event = os_current_event;

	// If no events, disable the event process
	if (!event) {
		__os_process_event_disable();
	}

	while (event) {
		do {
			/* If the queue is not empty */
			if (!os_event_is_empty(event)) {
				struct os_queue_event *queue_elt;
				/* Get the head element of the queue */
				queue_elt = os_queue_event_head(os_event_get_queue(event));
				/* Make sure the process is in pending state */
				if (__os_process_is_pending(queue_elt->proc)) {
					// Check if the event has been triggered
					status = event->desc.is_triggered(queue_elt->proc, event->args);
					if (status != OS_EVENT_NONE) {
						struct os_queue_event *current_elt;
						/* Update the event feedback variable */
						*queue_elt->event_triggered = event;
						// Remove the process from the event list;
						os_queue_event_pop(os_event_get_queue_ptr(event));
						// Activate the process
						__os_process_enable_naked(queue_elt->proc);
						/* Garbage collect, remove the other
						 * queue entries associated to this process
						 */
						current_elt = queue_elt->relation;
						while (current_elt != queue_elt) {
							os_queue_event_remove(current_elt);
							current_elt = current_elt->relation;
						}
					}
				}
				/* If the process is pending, pop this process out
				 * of the event list. This should never happen.
				 */
				else {
					while (true);
					/* Remove the not pending process from
					 * the list.
					 */
					os_queue_event_pop(os_event_get_queue_ptr(event));
					/* Continue processing the next process
					 * in the list.
					 */
					status = OS_EVENT_OK_CONTINUE;
				}
			}
			/* The process queue of the current even is empty,
			 * remove this event from the active event list.
			 */
			if (os_event_is_empty(event)) {
				status = OS_EVENT_NONE;
				os_event_pop(event);
			}
		} while (status == OS_EVENT_OK_CONTINUE);
		// Next event
		event = event->next;
	}

	// Call the scheduler
	os_switch_context(false);

	os_leave_critical();
}

struct __os_event_custom_function_args {
	bool (*trigger)(os_ptr_t);
	os_ptr_t args;
};

static enum os_event_status __os_event_custom_function_handler(struct os_process *proc,
		os_ptr_t args);
static enum os_event_status __os_event_custom_function_handler(struct os_process *proc,
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

void __os_event_register(struct os_event *event, struct os_queue_event *queue_elt,
		struct os_process *proc, struct os_event **event_triggered)
{
	os_queue_bidirectional_sort_t sort_fct;

	// Get the appropriate sorting function
#if CONFIG_OS_USE_PRIORITY == true
	sort_fct = os_queue_bidirectional_process_sort_priority;
#else
	sort_fct = (os_queue_bidirectional_sort_t) os_queue_sort_fifo;
#endif
	if (event->desc.sort) {
		sort_fct = event->desc.sort;
	}

	// Enable the event process if not done already, before messing up
	// with the process context
	__os_process_event_enable();

	// Add this event to the event list if not done already
	if (!os_event_is_enabled(event)) {
		__os_event_enable(event);
	}

	/* Assign the process to the queue element */
	queue_elt->proc = proc;
	/* Assign the variable to update when the event has been triggered */
	queue_elt->event_triggered = event_triggered;

	// Add the process to the event sorted process list
	os_queue_event_add_sort(os_event_get_queue_ptr(event), queue_elt, sort_fct);
}

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
	//__os_event_start(event);
	//__os_event_register(event, os_interrupt_get_process(interrupt));
	if (!is_critical) {
		os_leave_critical();
	}
}
#endif

#include <stdarg.h>

struct os_event *os_process_sleep(struct os_process *proc,
		struct os_queue_event *queue_elt, int nb_events, ...)
{
	int i;
	va_list ap;
	struct os_event *event, *event_triggered = NULL;
	/* Save the critical region status */
	bool is_critical = os_is_critical();

	/* Enter in a critical region if not already in */
	if (!is_critical) {
		os_enter_critical();
	}

	/* Disable the process (send it to sleep) */
	if (__os_process_is_enabled(proc)) {
		__os_process_disable_naked(proc);
	}

	/* Set the process status to pending */
	proc->status = OS_PROCESS_PENDING;

	/* No event has been triggered so far so update the feedback variable
	 * to NULL.
	 */
	event_triggered = NULL;

	/* Initialize relation pointer between the queue elements */
	queue_elt[0].relation = &queue_elt[0];

	va_start(ap, nb_events);
	i = nb_events;
	while (i--) {
		/* Get the next event from the list */
		event = va_arg(ap, struct os_event *);
		/* Start the event */
		__os_event_start(event, proc);
		/* Register the process in the active event list */
		__os_event_register(event, &queue_elt[i], proc, &event_triggered);
		/* Build a relation between the processes registered */
		if (i) {
			queue_elt[i].relation = queue_elt[0].relation;
			queue_elt[0].relation = &queue_elt[i];
		}
	}
	va_end(ap);

	/* If the process to be send to sleep is thye current process, stop it
	 * and use a garbage collector wipe out the extra events registered.
	 */
	if (proc == __os_process_get_current()) {
		/* Call the scheduler */
		os_switch_context(false);
	}

	/* Leave the critical region */
	if (!is_critical) {
		os_leave_critical();
	}

	/* Returns a pointer on the event which has woken up the process */
	return event_triggered;
}

#endif // CONFIG_OS_USE_EVENTS == true
