#ifndef __OS_EVENT_H__
#define __OS_EVENT_H__

enum os_event_status {
	OS_EVENT_NONE = -1,
	OS_EVENT_OK_STOP = 0,
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

struct os_event_descriptor {
	bool (*sort)(struct os_task_minimal *args1, struct os_task_minimal *args2);
	void (*start)(void *args);
	enum os_event_status (*is_triggered)(void *);
};

/*! \brief Event control
 * Events are used to wakeup a task or call an interrupt.
 * When a task is sleeping it will be removed from the active task list,
 * therefore the performance will not be decreased.
 *
 * Events are stored in a chain list as follow:
 * \code
 * E1 -> E2 -> E3 -> NULL
 * T1    T6    T3
 *       T7
 * \endcode
 */
struct os_event {
	struct os_event_descriptor desc;
	struct os_task_minimal *task;
	struct os_event *next;
	void *args;
};

/*! \brief Associate an event with a task.
 * The last task added will be the first task out.
 * Event can only be added to a task which is not active.
 */
static inline void os_event_add_task_lifo(struct os_event *event, struct os_task_minimal *task) {
	task->next = event->task;
	event->task = task;
}

/*! \brief Event scheduler
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
 * \param event A non-initialized event structure to hold the context of this
 * event
 * \param type The event description structure which defines the type of event
 * to be used.
 * \param args Argument which will be passed to the event descriptor functions.
 */
void os_event_create(struct os_event *event,
		const struct os_event_descriptor *descriptor, void *args);

/*! \brief Send a task to sleep.
 * The task will wake up uppon a specific event
 * \ingroup group_os_public_api
 */
void os_task_sleep(struct os_task *task, struct os_event *trigger);

#endif // __OS_EVENT_H__
