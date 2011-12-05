#ifndef __OS_CORE_H__
#define __OS_CORE_H__

#include "compiler.h"
#include "os_port.h"
#include "conf_os.h"

/*! \defgroup group_os Operating System
 * \brief This page contains all the documentation related to this operating
 * system (version \ref OS_VERSION).
 * - Preemptive and/or cooperative multi-tasking
 * - Task priority
 * - Hooks
 */

/*! \brief Current version of the operating system.
 */
#define OS_VERSION "0.1"

/*! \defgroup group_os_config Configuration
 * \brief Configuration flags available to customize the behavior of the OS.
 * \ingroup group_os
 * \{
 */

/*! \def CONFIG_OS_USE_TICK_COUNTER
 * \brief This configuration will enable the tick counter.
 */
#ifndef CONFIG_OS_USE_TICK_COUNTER
	#define CONFIG_OS_USE_TICK_COUNTER true
#endif

/*! \def CONFIG_OS_USE_16BIT_TICKS
 * \brief Use a 16-bits variable to define the tick counter.
 * \pre \ref CONFIG_OS_USE_TICK_COUNTER needs to be defined first.
 */
#ifndef CONFIG_OS_USE_16BIT_TICKS
	#define CONFIG_OS_USE_16BIT_TICKS false
#endif

/*! \def CONFIG_OS_SCHEDULER_TYPE
 * \brief Defines the type of scheduler to be used. Values can be found here
 * (\ref os_scheduler_type).
 *
 * This operating system can use cooperative and pre-emptive tasks together.
 * To allow \b only cooperative tasks, set the value to
 * \ref CONFIG_OS_SCHEDULER_COOPERATIVE.
 */
#ifndef CONFIG_OS_SCHEDULER_TYPE
	#define CONFIG_OS_SCHEDULER_TYPE CONFIG_OS_SCHEDULER_COOPERATIVE
#endif

/*! \defgroup os_scheduler_type Scheduler Type
 * \brief Configuration values for \ref CONFIG_OS_SCHEDULER_TYPE
 * \ingroup group_os_config
 */

/*!
 * \brief Use this option to use the OS in \b cooperative \b mode \b only.
 * \ingroup os_scheduler_type
 */
#define CONFIG_OS_SCHEDULER_COOPERATIVE 0
#ifndef CONFIG_OS_SCHEDULER_TYPE
	#error CONFIG_OS_SCHEDULER_TYPE must be set. It defines the type of scheduler to use.
#endif

/*! \def CONFIG_OS_TICK_HZ
 * \brief Set the tick frequency in Hz. This configuration is not used if
 * \ref CONFIG_OS_SCHEDULER_COOPERATIVE is chosen.
 */
#ifndef CONFIG_OS_TICK_HZ
	#define CONFIG_OS_TICK_HZ 1000
#endif

/*! \def CONFIG_OS_USE_PRIORITY
 * \brief Set this config to \b true in order to enable task priority support.
 */
#ifndef CONFIG_OS_USE_PRIORITY
	#define CONFIG_OS_USE_PRIORITY true
#endif

/*! \def CONFIG_OS_DEBUG
 * \brief Set this config to \b true to activate the \ref group_os_debug.
 */
#ifndef CONFIG_OS_DEBUG
	#define CONFIG_OS_DEBUG false
#endif

/*! \def CONFIG_OS_USE_MALLOC
 * \brief Use \ref os_malloc and \ref os_free to allocate/free the memory
 * for the stack
 */
#ifndef CONFIG_OS_USE_MALLOC
	#define CONFIG_OS_USE_MALLOC true
#endif

/*! \def CONFIG_OS_USE_CUSTOM_MALLOC
 * \brief Use a custom memory allocator. The user needs to define \ref os_malloc
 * and \ref os_free to allocate/free the memory
 */
#ifndef CONFIG_OS_USE_CUSTOM_MALLOC
	#define CONFIG_OS_USE_CUSTOM_MALLOC false
#endif

/*!
 * \}
 */

/*! \defgroup os_hook Hooks
 * \brief Allow the user to insert software hooks
 *
 * All the hooks are expected to be macros and should follow the following
 * assignement:
 * \code #define HOOK_OS_EXAMPLE() my_hook() \endcode
 * \ingroup group_os
 * \{
 */

/*! \def HOOK_OS_TICK
 * \brief This hook is called during each tick interrupt.
 */
#ifndef HOOK_OS_TICK
	#define HOOK_OS_TICK()
#endif

/*! \def HOOK_OS_IDLE
 * \brief This hook is called when no task is running and the application runs
 * in the IDLE loop.
 */
#ifndef HOOK_OS_IDLE
	#define HOOK_OS_IDLE()
#endif

/*! \def HOOK_OS_STACK_OVERFLOW
 * \brief This hook is called when a stack overflow error is detected.
 * \pre \ref CONFIG_OS_DEBUG must be set to true
 */
#ifndef HOOK_OS_STACK_OVERFLOW
	#define HOOK_OS_STACK_OVERFLOW()
#endif

/*!
 * \}
 */

/* Include OS modules */
#include "os_debug.h"

#if CONFIG_OS_USE_PRIORITY == true
/*! Priority of the task.
 * The lower get the most priority
 */
enum os_task_priority {
	OS_TASK_PRIORITY_1 = 0,
	OS_TASK_PRIORITY_2 = 1,
	OS_TASK_PRIORITY_3 = 2,
	OS_TASK_PRIORITY_4 = 3,
	OS_TASK_PRIORITY_5 = 4,
	OS_TASK_PRIORITY_10 = 9,
	OS_TASK_PRIORITY_20 = 19
};
#endif

/*! \brief Type to define a number of ticks
 */
#if CONFIG_OS_USE_16BIT_TICKS == true
typedef uint16_t os_tick_t;
#else
typedef uint32_t os_tick_t;
#endif

enum os_task_option {
	/*! Default options
	 */
	OS_TASK_DEFAULT = 0,
	/*! Disable the task before its execution.\n
	 * It can be enable at any time using \ref os_task_enable
	 */
	OS_TASK_DISABLE = 1,
	/*! Use a custom stack for this task. The user must previously allocate
	 * memory for \ref os_task::stack. This option is available only if
	 * \ref CONFIG_OS_USE_MALLOC is set.
	 */
	OS_TASK_USE_CUSTOM_STACK = 2,
};

/*! This structure represents the minimalistic task context
 */
struct os_task_minimal {
	/*! Stack pointer. Will always be the 1rst element of this structure,
	 * to ensure the best optimization.
	 */
	void *sp;
	/*! Pointer of the next task in the list.
	 * Active tasks are registered within a chain list.
	 */
	struct os_task *next;
#if CONFIG_OS_USE_PRIORITY == true
	/*! Priority of the stack. The priority is the ratio
	 */
	enum os_task_priority priority;
	/*! Use to manage the task priorities
	 */
	enum os_task_priority priority_counter;
#endif
};

/*! Structure holding the context of a task
 */
struct os_task {
	/*! Minimal stack context
	 */
	struct os_task_minimal core;
	/*! A pointer on a memory space reserved for the stack
	 */
	uint8_t *stack;
	/*! Task options
	 */
	enum os_task_option options;
};

/*! Task function prototype
 * \param args Arguments passed to the task in a form of an empty pointer
 */
typedef void (*task_ptr_t)(void *args);

/*! This function will define the rules to change the task.
 * \return The new task context
 */
#if CONFIG_OS_USE_PRIORITY == true
static inline void os_task_scheduler(void) {
	extern void os_task_scheduler_priority(void);
	os_task_scheduler_priority();
}
#else
static inline void os_task_scheduler(void) {
	extern struct os_task *os_current_task;
	os_current_task = os_current_task->core.next;
}
#endif

/*! \defgroup os_public_api Public API
 * \brief Public application interface.
 * \ingroup group_os
 * \{
 */

/*! \brief Allocate memory for the stack
 * This macro can be used with \ref OS_TASK_USE_CUSTOM_STACK in order to
 * manually allocate some memory for the stack.
 * Here is an example code to use this macro:
 * \code
 * void my_func(void *args)
 * {
 *	...
 * }
 * static OS_MALLOC_STACK(my_stack, 1024);
 * struct os_task my_task;
 * my_task.stack = my_stack;
 * os_task_add(&my_task, my_func, NULL, 0, OS_TASK_USE_CUSTOM_STACK);
 * \endcode
 * \param stack_symbol The symbol name used to refer to this stack
 * \param stack_size The size of the stack in bytes
 */
#define OS_MALLOC_STACK(stack_symbol, stack_size) \
		uint8_t (stack_symbol)[(stack_size)]

/*! \brief Convert a delay in milliseconds to a number of ticks.
 * \param time_ms The time (in ms) to be converted
 * \return The number of ticks
 */
#define OS_MS_TO_TICK(time_ms) \
		(((time_ms) * CONFIG_OS_TICK_HZ) / 1000)

/*! \brief Convert a delay in seconds to a number of ticks.
 * \param time_s The time (in s) to be converted
 * \return The number of ticks
 */
#define OS_S_TO_TICK(time_s) \
		((time_s) * CONFIG_OS_TICK_HZ)

/*! \brief Create and add a new task.
 * \param task A pointer on an empty structure which will contain the context of
 * the current task.
 * \param task_ptr Entry point of the task to be run.
 * \param args Arguments to pass to the task
 * \param stack_size The size of the stack in byte
 * \param options Specific options for the task (see \ref os_task_option)
 * \return true if the task has been correctly registered, false otherwise.
 */
bool os_task_add(struct os_task *task, task_ptr_t task_ptr, void *args,
		int stack_size, enum os_task_option options);

/*! \brief Delete a task
 * \param task The task to be deleted
 */
void os_task_delete(struct os_task *task);

/*! \brief Enable the execution a task
 * \param task The task to be enabled
 */
void os_task_enable(struct os_task *task);

/*! \brief Disable the execution of a task
 * \param task The task to be disabled
 */
void os_task_disable(struct os_task *task);

/*! \brief Check wether a task is enabled or not
 * \param task The task to be checked
 * \return true if enabled, false otherwise
 */
bool os_task_is_enabled(struct os_task *task);

/*! \brief Call the scheduler to switch to a new task
 * This function is usefull for cooperative task swiching
 */
void os_task_yield(void);

#if CONFIG_OS_USE_TICK_COUNTER == true
/*! \brief Block the execution of a task until a number of ticks have passed.
 * \ref CONFIG_OS_TICK_HZ can be used to estimate a time delay.
 * \param tick_nb The number of ticks to wait for
 * \pre \ref CONFIG_OS_USE_TICK_COUNTER needs to be set first.
 */
void os_task_delay(os_tick_t tick_nb);
#endif

/*! Send a task to sleep.
 * The task will wake up uppon a specific event
 */
// void os_task_sleep(struct os_task *task, struct os_event *trigger);

/*! \brief Start the task scheduling process
 * \param ref_hz The frequency which runs the peripheral to generate
 * the ticks. Usually this frequency is equal to the CPU frequency.
 */
static inline void os_start(uint32_t ref_hz) {
	extern void os_setup_scheduler(uint32_t);
#if CONFIG_OS_SCHEDULER_TYPE != CONFIG_OS_SCHEDULER_COOPERATIVE
	// Setup the scheduler
	os_setup_scheduler(ref_hz);
#endif
	// Launch the scheduler
	os_task_yield();
	// Idle loop
	while (true) {
#if HOOK_OS_IDLE
		HOOK_OS_IDLE();
#endif
	}
}

/*! \brief Get the current running task
 * \return the current task. NULL if none is running.
 */
struct os_task *os_task_current(void);

#if CONFIG_OS_USE_PRIORITY == true
/*! \brief Change the priority of a task
 * \param task The task which needs some update
 * \param priority The new priority
 * \pre \ref CONFIG_OS_USE_PRIORITY needs to be set first
 */
static inline void os_task_set_priority(struct os_task *task, enum os_task_priority priority) {
	os_enter_critical();
	task->core.priority = priority;
	task->core.priority_counter = priority;
	os_leave_critical();
}
/*! \brief Get the priority of a task
 * \param task The task which priority is requested
 * \return The task priority
 */
static inline enum os_task_priority os_task_get_priority(struct os_task *task) {
	return task->core.priority_counter;
}
#endif

/*!
 * \}
 */

/*! \defgroup os_port_group Porting functions
 * \brief Functions which should be implemented to port this operating system onto
 * another platform.
 * \ingroup group_os
 * \{
 */

/*!
 * \fn void os_enter_critical(void)
 * \brief Enter a critical zone which must not be interrupted by any other
 * tasks\n
 * This function will basically disable interrupts
 */

/*!
 * \fn void os_leave_critical(void)
 * \brief Exit a critical zone\n
 * This function will basically enable interrupts
 */

/*!
 * \brief Setup the task scheduler interrupt.
 * \param ref_hz The reference frequency used to clock the peripheral which will
 * generate the tick interrupt. Usually this frequency is equal to the CPU
 * frequency.
 */
void os_setup_scheduler(uint32_t ref_hz);

/*!
 * \fn void os_task_switch_context(void)
 * \brief Context switch for a task.\n
 * Function used to schedule and switch between the tasks.
 */

/*!
 * \fn void os_task_switch_context_int_handler(void)
 * \brief Context switch for a task.\n
 * Interrupt handler which is used to schedule and switch between the tasks.
 */

/*!
 * \fn void *os_malloc(int stack_size)
 * \brief Allocate some memory for the stack of a task
 * \param stack_size The size in byte of the stack
 * \return The pointer of the memory allocated, NULL in case of an error.
 */
#if CONFIG_OS_USE_CUSTOM_MALLOC == false
static inline void *os_malloc(int stack_size) {
	return malloc(stack_size);
}
#endif

/*!
 * \fn void os_free(void *ptr)
 * \brief Free memory previously allocated by \ref os_malloc
 * \param ptr The memory to free
 */
#if CONFIG_OS_USE_CUSTOM_MALLOC == false
static inline void os_free(void *ptr) {
	free(ptr);
}
#endif

/*!
 * \brief Load the context of a task into the stack. this is the inital process
 * which will setup the stack before entering in the task function.
 * \param task The task
 * \param task_ptr Pointer of the entry point of the task
 * \param args Parameters to pass to the task
 * \return true in case of success, false otherwise
 */
bool os_task_context_load(struct os_task *task, task_ptr_t task_ptr,
		void *args);

/*!
 * \brief This function must be called inside the
 * \ref os_task_switch_context_int_handler function in order to switch task
 * context.
 */
static inline void os_task_switch_context_int_handler_hook(void) {
#if CONFIG_OS_USE_TICK_COUNTER == true
	extern volatile os_tick_t tick_counter;
	// Update the tick counter
	tick_counter++;
#endif
#if CONFIG_OS_DEBUG
	HOOK_OS_DEBUG_TICK();
#endif
#if HOOK_OS_TICK
	HOOK_OS_TICK();
#endif
	// Task switch context
	os_task_scheduler();
}

/*!
 * \brief This function must be called inside the
 * \ref os_task_switch_context function in order to switch task context.
 */
static inline void os_task_switch_context_hook(void) {
	// Task switch context
	os_task_scheduler();
}

/*!
 * \}
 */

#endif // __OS_CORE_H__
