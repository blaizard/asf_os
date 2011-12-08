#ifndef __OS_INTERRUPT_H__
#define __OS_INTERRUPT_H__

/*! \brief Software interrupt structure
 */
struct os_interrupt {
	/*! \brief Task context of the interrupt
	 */
	struct os_task_minimal core;
	/*! \brief Pointer in the interrupt handler
	 */
	task_ptr_t task_ptr;
	/*! \brief Arguments to pass to the interrupt handler
	 */
	void *args;
};

/*! \ingroup group_os_config
 *
 * \{
 */

/*! \def CONFIG_OS_USE_SW_INTERRUPTS
 * \brief Use this option to enable software interrupts.
 */
#ifndef CONFIG_OS_USE_SW_INTERRUPTS
	#define CONFIG_OS_USE_SW_INTERRUPTS false
#endif

/*! \def CONFIG_OS_INTERRUPT_DEFAULT_PRIORITY
 * \brief Default priority assgined to an interrupt
 */
#ifndef CONFIG_OS_INTERRUPT_DEFAULT_PRIORITY
	#define CONFIG_OS_INTERRUPT_DEFAULT_PRIORITY OS_PRIORITY_1
#endif

/*!
 * \}
 */

/*! \name Software Interrupts
 *
 * A software interrupt is a task which runs from the application context and
 * it cannot be interrupted by the scheduler.
 * Software interrupts are scheduled with the task scheduler. They inherit
 * from the same priority scheme than a task.
 *
 * \pre \ref CONFIG_OS_USE_SW_INTERRUPTS needs to be set
 *
 * \ingroup group_os_public_api
 *
 * \{
 */

/*! \brief Setup a software interrupt
 * \ingroup group_os_public_api
 * \param interrupt The non-initialized structure to hold the context of the
 * software interrupt
 * \param task_ptr A pointer on the interrupt handler (a interrupt handler is a
 * normal function which follow the \ref task_ptr_t prototype)
 * \param args Arguments to pass to the inerrupt handler
 */
void os_interrupt_setup(struct os_interrupt *interrupt, task_ptr_t task_ptr,
		void *args);

/*! \brief Trigger a software interrupt.
 * \ingroup group_os_public_api
 * \param interrupt The interrupt to trigger
 * \pre The interrupt must be previously setup with \ref os_interrupt_setup
 */
static inline void os_interrupt_trigger(struct os_interrupt *interrupt) {
	os_task_enable((struct os_task *) interrupt);
}

#if CONFIG_OS_USE_PRIORITY == true
/*! \brief Change the priority of a software interrupt
 * \ingroup group_os_public_api
 * \param interrupt The interrupt which needs some update
 * \param priority The new priority
 * \pre \ref CONFIG_OS_USE_PRIORITY needs to be set first
 */
static inline void os_interrupt_set_priority(struct os_interrupt *interrupt, enum os_priority priority) {
	os_task_set_priority((struct os_task *) interrupt, priority);
}
/*! \brief Get the priority of a software interrupt
 * \ingroup group_os_public_api
 * \param interrupt The interrupt which priority is requested
 * \return The interrupt priority
 */
static inline enum os_priority os_interrupt_get_priority(struct os_interrupt *interrupt) {
	return os_task_get_priority((struct os_task *) interrupt);
}
#endif

/*!
 * \}
 */

/*!
 * \ingroup group_os_internal_api
 * \{
 */

/*! \brief Software interrupt handler
 * \ingroup group_os_internal_api
 * \param args A pointer on a \ref os_interrupt structure
 */
void __os_interrupt_handler(void *args);

/*! \brief Test if the current running task is an interrupt.
 * \ingroup group_os_internal_api
 * \return true if the task is an interrupt, false otherwise.
 */
static inline bool __os_task_is_interrupt(void) {
	extern bool os_interrupt_flag;
	return os_interrupt_flag;
}

/*!
 * \}
 */

#if CONFIG_OS_USE_SW_INTERRUPTS == true
	#define OS_SCHEDULER_INTERRUPT_PRE_HOOK() \
		do { \
			extern bool os_interrupt_flag; \
			extern struct os_task_minimal *os_current_task; \
			if (os_interrupt_flag) { \
				os_interrupt_flag = false; \
				os_current_task->sp = NULL; \
			} \
		} while (false)

	#define OS_SCHEDULER_INTERRUPT_POST_HOOK() \
		do { \
			extern bool os_interrupt_flag; \
			if (os_current_task->sp) \
				return os_current_task; \
			os_interrupt_flag = true; \
			os_current_task->sp = os_app.sp; \
			os_task_context_load(os_current_task, \
					__os_interrupt_handler, \
					(void *) os_current_task); \
		} while (false)
#endif

#endif // __OS_INTERRUPT_H__
