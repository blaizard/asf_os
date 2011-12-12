/*! \file
 * \brief eeOS Events
 * \author Blaise Lengrand (blaise.lengrand@gmail.com)
 * \version 0.1
 * \date 2011
 *
 * \section eeos_license License
 * \ref eeos is provided in source form for FREE evaluation, for
 * educational use or for peaceful research. If you plan on using \ref eeos in a
 * commercial product you need to contact the author to properly license
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
	/*! \brief An event has been triggered for this task. If another
	 * task is sharing the same event, there is no need to test if this
	 * event is also valid for it. In other word, this event is only valid
	 * for 1 task.
	 */
	OS_EVENT_OK_STOP = 0,
	/*! \brief An event has been triggered for this task. If another
	 * task is sharing the same event, we might want to also chech if this
	 * event is also valid. In other word, this event might be valid for
	 * more than 1 task.
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
	/*! \brief Sorting function. This function compare 2 tasks.
	 * The following helper functions are available:
	 * - \ref os_event_sort_fifo
	 * - \ref os_event_sort_lifo
	 * \param task1 The first task used in the comparison
	 * \param task2 The second task used in the comparison
	 * \return true if task1 should be placed before task2, false otherwise.
	 */
	bool (*sort)(struct os_task_minimal *task1, struct os_task_minimal *task2);
	/*! \brief This function will handle the setup of the event. For
	 * example, if the event is a timer, the timer will start to run after
	 * the call of this function.
	 * \param args Argument passed to the event during its creation
	 */
	void (*start)(void *args);
	/*! \brief Check the status of an event
	 * \param args Argument passed to the event during its creation
	 * \return The current event status (\ref os_event_status)
	 */
	enum os_event_status (*is_triggered)(void *args);
};

/*! \brief Event control
 * Events are used to wakeup a task or call an interrupt so an event
 * is always associated to a task/interrupt.
 * When a task is sleeping it will be removed from the active task list,
 * therefore the performance will not be decreased.
 *
 * Events are stored in a chain list as follow, where \i E are events and \i T
 * are tasks:
 * \code
 * E1 -> E2 -> E3 -> NULL
 * T1    T6    T3
 *       T7
 * \endcode
 * When an event has no task, it is removed from the active event list.
 */

/*! \brief Event structure
 */
struct os_event {
	/*! \brief This structure contains the event descriptor functions. These
	 * functions will define the behavior of this event.
	 */
	struct os_event_descriptor desc;
	/*! \brief This is the starting point of the task chain list associated
	 * with this event. The last task is followed by a NULL pointer.
	 */
	struct os_task_minimal *task;
	/*! \brief Next event in the chain list. Last event is followed by a
	 * NULL pointer.
	 */
	struct os_event *next;
	/*! \brief Extra arguments used to define this event.
	 */
	void *args;
};

/*!
 * \name Event Helper Functions
 * \{
 */
/*! \brief Helper function used to define the order of a new task added to an
 * event. This function will add them so that the \b first \b in will be the
 * \b first \b out. This function must be used with
 * \ref os_event_descriptor::sort.
 * \ref task1 First task to be compared
 * \ref task2 Second task to be compared
 * \return true id task1 is < task2.
 */
bool os_event_sort_fifo(struct os_task_minimal *task1,
		struct os_task_minimal *task2);
/*! \brief Helper function used to define the order of a new task added to an
 * event. This function will add them so that the \b last \b in will be the
 * \b first \b out. This function must be used with
 * \ref os_event_descriptor::sort.
 * \ref task1 First task to be compared
 * \ref task2 Second task to be compared
 * \return true id task1 is < task2.
 */
bool os_event_sort_lifo(struct os_task_minimal *task1,
		struct os_task_minimal *task2);
/*!
 * \}
 */

/*! \brief Event scheduler
 * \ingroup group_os_internal_api
 * \return true if at least 1 event is present in the event list, false
 * otherwise.
 */
#if CONFIG_OS_USE_EVENTS == true
bool os_event_scheduler(void);
#else
static inline bool os_event_scheduler(void) {
	return false;
}
#endif

/*! \brief Create a new event
 * \ingroup group_os_internal_api
 * \param event A non-initialized event structure to hold the context of this
 * event
 * \param type The event description structure which defines the type of event
 * to be used.
 * \param args Argument which will be passed to the event descriptor functions.
 */
void os_event_create(struct os_event *event,
		const struct os_event_descriptor *descriptor, void *args);

/*! \brief Associate a task with an event and enable the event
 * \ingroup group_os_internal_api
 * \param event The event to receive the task
 * \param task The task to add
 */
void __os_event_register(struct os_event *event, struct os_task_minimal *task);

#if CONFIG_OS_USE_EVENTS == true
	/*! This macro will test if a alternative current task is needed.
	 * It will restore the next current task previously erased by
	 * os_task_sleep, call the scheduler and clear the flag to prevent the
	 * next iteration to use it.
	 */
	#define OS_SCHEDULER_POST_EVENT_HOOK() \
		do { \
			extern struct os_task_minimal os_event_alternate_task; \
			extern struct os_task_minimal *os_current_task; \
			if (os_event_alternate_task.next) { \
				struct os_task_minimal *current_task; \
				os_current_task = &os_event_alternate_task; \
				current_task = os_task_scheduler(); \
				os_event_alternate_task.next = NULL; \
				return current_task;\
			} \
		} while (false)
#endif

#endif // __OS_EVENT_H__
