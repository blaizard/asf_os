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

#ifndef __OS_EVENT_H__
#define __OS_EVENT_H__

/*! \brief Status returned by an event
 */
enum os_event_status {
	/*! \brief No event has occured
	 */
	OS_EVENT_NONE = -1,
	/*! \brief An event has been triggered for this process. If another
	 * process is sharing the same event, there is no need to test if this
	 * event is also valid for it. In other word, this event is only valid
	 * for 1 process.
	 */
	OS_EVENT_OK_STOP = 0,
	/*! \brief An event has been triggered for this process. If another
	 * process is sharing the same event, we might want to also chech if this
	 * event is also valid. In other word, this event might be valid for
	 * more than 1 process.
	 */
	OS_EVENT_OK_CONTINUE = 1,
};

/*! \ingroup group_os_config
 *
 * \{
 */

/*! \def CONFIG_OS_USE_EVENTS
 * \brief Use this option to enable event support.
 */
#ifndef CONFIG_OS_USE_EVENTS
	#define CONFIG_OS_USE_EVENTS false
#endif

/*!
 * \}
 */

/*! \brief Event descriptor
 */
struct os_event_descriptor {
	/*! \brief Sorting function. This function compare 2 processes.
	 * The following helper functions are available:
	 * - \ref os_event_sort_fifo
	 * - \ref os_event_sort_lifo
	 * \note This function is optional. By default the new process will added
	 * using the \ref os_event_sort_fifo algorithm or the
	 * \ref os_event_sort_priority alorithm, depending if priorities are
	 * enabled or not.
	 * \param proc1 The first process used in the comparison
	 * \param proc2 The second process used in the comparison
	 * \return true if proc1 should be placed before proc2, false otherwise.
	 */
	bool (*sort)(struct os_process *proc1, struct os_process *proc2);
	/*! \brief This function will handle the setup of the event. For
	 * example, if the event is a timer, the timer will start to run after
	 * the call of this function.
	 * \param args Argument passed to the event during its creation
	 */
	void (*start)(os_ptr_t args);
	/*! \brief Check the status of an event
	 * \note This function is optional.
	 * \param proc The process of the process which is intented to triggered
	 * \param args Argument passed to the event during its creation
	 * \return The current event status (\ref os_event_status)
	 */
	enum os_event_status (*is_triggered)(struct os_process *proc, os_ptr_t args);
};

/*! \addtogroup group_os
 * \section section_os_event Events
 *
 * Events (\ref os_event) are used to wake up or call one or multiple processes.
 * When a process is sleeping it will be removed from the active process list,
 * therefore the performance will not be decreased.
 *
 * Events are stored in a chain list as follow, where \i E are events and \i P
 * are processes:
 * \code
 *  E1 -> E2 -> E3 -> NULL
 *  P1    P6    P3
 * NULL   P7   NULL
 *       NULL
 * \endcode
 * When an event has no process, it is removed from the active event list.
 */

/*! \brief Event structure
 */
struct os_event {
	/*! \brief This structure contains the event descriptor functions. These
	 * functions will define the behavior of this event.
	 */
	struct os_event_descriptor desc;
	/*! \brief This is the starting point of the process chain list associated
	 * with this event. The last process is followed by a NULL pointer.
	 */
	struct os_process *proc;
	/*! \brief Next event in the chain list. Last event is followed by a
	 * NULL pointer.
	 */
	struct os_event *next;
	/*! \brief Extra arguments used to define this event.
	 */
	os_ptr_t args;
};

/*! \name Events
 *
 * Set of functions to create and manage events
 *
 * \{
 */

/*! \brief Create a custom event from a function
 * \ingroup group_os_public_api
 * \param event The un-initialized event structure to be filled
 * \param trigger The boolean function to trigger the event. The event will be
 * triggered when this function returns true.
 * \param args Arguments to pass to the trigger function
 */
void os_event_create_from_function(struct os_event *event,
		bool (*trigger)(os_ptr_t), os_ptr_t args);

/*!
 * \}
 */

/*!
 * \name Event Helper Functions
 * \{
 */
/*! \brief Helper function used to define the order of a new process added to an
 * event. This function will add them so that the \b first \b in will be the
 * \b first \b out. This function must be used with
 * \ref os_event_descriptor::sort.
 */
bool os_event_sort_fifo(struct os_process *proc1, struct os_process *proc2);
/*! \brief Helper function used to define the order of a new process added to an
 * event. This function will add them so that the \b last \b in will be the
 * \b first \b out. This function must be used with
 * \ref os_event_descriptor::sort.
 */
bool os_event_sort_lifo(struct os_process *proc1, struct os_process *proc2);

/*! \brief Helper function used to define the order of a new process added to an
 * event. This function will add them so that the highest priority process will be
 * at the head of the list. This function must be used with
 * \ref os_event_descriptor::sort.
 * \pre \ref CONFIG_OS_USE_PRIORITY must be set
 */
bool os_event_sort_priority(struct os_process *proc1, struct os_process *proc2);
/*!
 * \}
 */

/*! \brief Event scheduler
 * \ingroup group_os_internal_api
 * \return true if at least 1 event is present in the event list, false
 * otherwise.
 */
void os_event_scheduler(void);

/*! \brief Create a new event
 * \ingroup group_os_internal_api
 * \param event A non-initialized event structure to hold the context of this
 * event
 * \param type The event description structure which defines the type of event
 * to be used.
 * \param args Argument which will be passed to the event descriptor functions.
 */
void os_event_create(struct os_event *event,
		const struct os_event_descriptor *descriptor, os_ptr_t args);

/*! \brief Associate a process with an event and enable the event
 * \ingroup group_os_internal_api
 * \param event The event to receive the process
 * \param proc The process to add
 */
void __os_event_register(struct os_event *event, struct os_process *proc);

#if CONFIG_OS_USE_EVENTS == true
	/*! This macro will test if a alternative current process is needed.
	 * It will restore the next current process previously erased by
	 * os_task_sleep, call the scheduler and clear the flag to prevent the
	 * next iteration to use it.
	 */
	#define OS_SCHEDULER_POST_EVENT_HOOK() \
		do { \
			extern struct os_process __os_event_alternate_proc; \
			extern struct os_process *__os_current_process; \
			if (__os_event_alternate_proc.next) { \
				struct os_process *current_proc; \
				__os_current_process = &__os_event_alternate_proc; \
				current_proc = os_scheduler(); \
				__os_event_alternate_proc.next = NULL; \
				return current_proc;\
			} \
		} while (false)
#endif

#endif // __OS_EVENT_H__
