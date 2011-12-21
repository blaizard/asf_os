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
	 * - \ref os_queue_sort_fifo
	 * - \ref os_queue_sort_lifo
	 * \note This function is optional. By default the new process will added
	 * using the \ref os_queue_sort_fifo algorithm or the
	 * \ref os_queue_process_sort_priority alorithm, depending if priorities
	 * are enabled or not.
	 * \param a The first process used in the comparison
	 * \param b The second process used in the comparison
	 * \return true if proc1 should be placed before proc2, false otherwise.
	 */
	bool (*sort)(struct os_queue *a, struct os_queue *b);
	/*! \brief This function will handle the setup of the event. For
	 * example, if the event is a timer, the timer will start to run after
	 * the call of this function.
	 * \param proc The process associated with this event
	 * \param args Argument passed to the event during its creation
	 */
	void (*start)(struct os_process *proc, os_ptr_t args);
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

OS_QUEUE_DEFINE(event, struct os_process *proc; struct os_event **event_triggered;)

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
	struct os_queue_event *queue;
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

static inline bool os_event_is_enabled(struct os_event *event) {
	return (bool) (event->queue);
}

struct os_event *os_process_sleep(struct os_process *proc,
		struct os_queue_event *queue_elt, int nb_events, ...);

/*! \brief Associate a process with an event and enable the event
 * \ingroup group_os_internal_api
 * \param event The event to receive the process
 * \param queue_elt A unitialized queue structure used to hold the process
 * in the queue.
 * \param proc The process to add
 * \param event_triggered A pointer to update which will reflect the event that
 * has been triggered
 */
void __os_event_register(struct os_event *event, struct os_queue_event *queue_elt,
		struct os_process *proc, struct os_event **event_triggered);

#endif // __OS_EVENT_H__
