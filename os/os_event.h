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
	/*! \brief Sorting function. This function compare 2 tasks.
	 * The following helper functions are available:
	 * - \ref os_event_sort_fifo
	 * - \ref os_event_sort_lifo
	 * \param task1 The first task used in the comparison
	 * \param task2 The second task used in the comparison
	 * \return true if task1 should be placed before task2, false otherwise.
	 */
	bool (*sort)(struct os_task_minimal *task1, struct os_task_minimal *task2);
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

/*!
 * \name Event Helper Functions
 * \{
 */
bool os_event_sort_fifo(struct os_task_minimal *task1,
		struct os_task_minimal *task2);
bool os_event_sort_lifo(struct os_task_minimal *task1,
		struct os_task_minimal *task2);
/*!
 * \}
 */

/*! \brief Associate an event with a task.
 * The last task added will be the first task out.
 * Event can only be added to a task which is not active.
 */
static inline void os_event_add_task_lifo(struct os_event *event, struct os_task_minimal *task) {
	task->next = event->task;
	event->task = task;
}

static inline void os_event_add_task_fifo(struct os_event *event, struct os_task_minimal *task) {
	struct os_task_minimal **last_task = &event->task;
	// Look for the last task in the list
	while (*last_task) {
		last_task = &(*last_task)->next;
	}
	*last_task = task;
	task->next = NULL;
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

/*! \brief Associate a task with an event
 * \ingroup group_os_public_api
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
