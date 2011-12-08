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

/*! \brief Event control
 * Events are used to wakeup a task. If a task is sleeping it will be removed
 * from the active task list and then the performance will not be impacted.
 * Events will be monitored and once an event occurs, the associated task will
 * wake up.
 */
struct os_event {
	enum os_event_status (*is_triggered)(void *);
	struct os_task_minimal *task;
	struct os_event *prev;
	struct os_event *next;
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
 * \return true if at least 1 event is present in the event list,
 * false otherwise.
 */
bool os_event_scheduler(void);

/*! \brief Send a task to sleep.
 * The task will wake up uppon a specific event
 * \ingroup group_os_public_api
 */
// void os_task_sleep(struct os_task *task, struct os_event *trigger);

#endif // __OS_EVENT_H__
