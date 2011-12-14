#ifndef __OS_TASK_H__
#define __OS_TASK_H__

enum os_task_option {
	/*! \brief Default options
	 */
	OS_TASK_DEFAULT = 0,
	/*! \brief Disable the task before its execution.\n
	 * It can be enable at any time using \ref os_task_enable
	 */
	OS_TASK_DISABLE = 1,
	/*! \brief Use a custom stack for this task. The user must previously allocate
	 * memory for \ref os_task::stack. This option is available only if
	 * \ref CONFIG_OS_USE_MALLOC is set.
	 */
	OS_TASK_USE_CUSTOM_STACK = 2,
};

/*! Structure holding the context of a task
 */
struct os_task {
	/*! \brief Minimal context
	 */
	struct os_process core;
	/*! \brief A pointer on a memory space reserved for the stack
	 */
	uint8_t *stack;
	/*! \brief Task options
	 */
	enum os_task_option options;
};

/*! \name Tasks
 *
 * Set of functions to manage a task
 *
 * \{
 */

/*! \brief Create a new task. By default, the new task will be automatically
 * added to the active task list unless specified.
 * \ingroup group_os_public_api
 * \param task A pointer on an empty structure which will contain the context of
 * the current task.
 * \param task_ptr Entry point of the task to be run.
 * \param args Arguments to pass to the task
 * \param stack_size The size of the stack in byte
 * \param options Specific options for the task (see \ref os_task_option)
 * \return true if the task has been correctly registered, false otherwise.
 */
bool os_task_create(struct os_task *task, os_proc_ptr_t task_ptr, os_ptr_t args,
		int stack_size, enum os_task_option options);

#if CONFIG_OS_USE_TICK_COUNTER == true
/*! \brief Block the execution of a task until a number of ticks have passed.
 * \ingroup group_os_public_api
 * \ref CONFIG_OS_TICK_HZ can be used to estimate a time delay.
 * \param tick_nb The number of ticks to wait for
 * \pre \ref CONFIG_OS_USE_TICK_COUNTER needs to be set first.
 * \warning This functon needs the preemptive scheduler to run. Therefore, it
 * cannot be used inside an interrupt or any other piece of code where the
 * tick interrupt is disabled.
 */
void os_task_delay(os_tick_t tick_nb);
#endif

/*! \brief Get the task associated with a process
 * \ingroup group_os_public_api
 * \param proc the process
 * \return The task pointer
 */
static inline struct os_task *os_task_from_process(struct os_process *proc) {
	return container_of(proc, struct os_task, core);
}

/*! \brief Get the task process
 * \ingroup group_os_public_api
 * \param task The task
 * \return The process of the task
 */
static inline struct os_process *os_task_get_process(struct os_task *task) {
	return &task->core;
}

#if CONFIG_OS_USE_PRIORITY == true
/*! \brief Set a priority to a task
 * \ingroup group_os_public_api
 * \param task The task
 * \param priority The priority to set
 */
static inline void os_task_set_priority(struct os_task *task, enum os_priority priority) {
	os_process_set_priority(os_task_get_process(task), priority);
}
/*! \brief Get the priority of a task
 * \ingroup group_os_public_api
 * \param task The task
 * \return The priority of the task
 */
static inline enum os_priority os_task_get_priority(struct os_task *task) {
	return os_process_get_priority(os_task_get_process(task));
}
#endif

/*! \brief Delete a task
 * \ingroup group_os_public_api
 * \param task The task to be deleted
 */
static inline void os_task_delete(struct os_task *task) {
	os_process_disable(os_task_get_process(task));
	// Free the task stack if needed
	if (!(task->options & OS_TASK_USE_CUSTOM_STACK)) {
		extern void os_free(os_ptr_t ptr);
		os_free(task->stack);
	}
}

/*! \brief Enable the execution a task
 * \ingroup group_os_public_api
 * \param task The task to be enabled
 */
static inline void os_task_enable(struct os_task *task) {
	os_process_enable(os_task_get_process(task));
}

/*! \brief Disable the execution of a task
 * \ingroup group_os_public_api
 * \param task The task to be disabled
 */
static inline void os_task_disable(struct os_task *task) {
	os_process_disable(os_task_get_process(task));
}

/*! \brief Check wether a task is enabled or not
 * \ingroup group_os_public_api
 * \param task The task to be checked
 * \return true if enabled, false otherwise
 */
static inline bool os_task_is_enabled(struct os_task *task) {
	return os_process_is_enabled(os_task_get_process(task));
}

/*! \brief Get the current running task
 * \ingroup group_os_public_api
 * \return the current task. NULL if none is running.
 */
struct os_task *os_task_get_current(void);

#if CONFIG_OS_USE_EVENTS == true
/*! \brief Send the task to sleep and wake it up uppon a specific event
 * \ingroup group_os_public_api
 * \param task The task to send to sleep
 * \param event The event used to wakeup the task
 * \pre \ref CONFIG_OS_USE_EVENTS needs to be set
 */
void os_task_sleep(struct os_task *task, struct os_event *event);
#endif

/*!
 * \}
 */

#endif // __OS_TASK_H__
