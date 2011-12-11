/*! \file
 * \brief eeOS Core
 * \author Blaise Lengrand (blaise.lengrand@gmail.com)
 * \version 0.1
 * \date 2011
 *
 * \section eeos_license License
 * eeOS is provided in source form for FREE evaluation, for
 * educational use or for peaceful research. If you plan on using eeOS in a
 * commercial product you need to contact the author to properly license
 * its use in your product. The fact that the  source is provided does
 * NOT mean that you can use it without paying a licensing fee.
 */

#ifndef __OS_CORE_H__
#define __OS_CORE_H__

#include "os_port.h"
#include "conf_os.h"

/*! \defgroup group_os Embedded Event-based Operating System (eeOS)
 * \brief This page contains all the documentation related to this operating
 * system (\ref OS_VERSION).
 * - Preemptive and/or cooperative round-robin multi-tasking
 * - Very scalable
 * - Task priority
 * - Hook points
 * - Software interrupt with priority
 * - Doxygen documented
 * - Advanced event system
 * - Semaphores with priority inheritance
 *
 * All the actives tasks are stored in a chain list.
 * The current task is the task pointed by \ref os_current_task.
 * At the begining, when the task scheduler is not running the current process
 * is not a task so its information need to be storted in a specific context.
 * This context is \ref os_app. Therefore, at the begining the active task chain
 * list looks like this:
 * os_current_task -> os_app -> task1 -> task2 -> task1 -> task2 -> ...
 * During normal execution, os_app is not part of the chain list anymore:
 * os_current_task -> task1 -> task2 -> task1 -> task2 -> ...
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
#if CONFIG_OS_SCHEDULER_TYPE == CONFIG_OS_SCHEDULER_COOPERATIVE && \
		CONFIG_OS_USE_TICK_COUNTER == true
	#error The tick counter cannot be used when only cooperative scheduler\
			is used. CONFIG_OS_SCHEDULER_TYPE must be set to\
			something else than CONFIG_OS_SCHEDULER_COOPERATIVE.
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

/*! \def CONFIG_OS_TASK_DEFAULT_PRIORITY
 * \brief Default priority assgined to a task
 */
#ifndef CONFIG_OS_TASK_DEFAULT_PRIORITY
	#define CONFIG_OS_TASK_DEFAULT_PRIORITY OS_PRIORITY_1
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

#if CONFIG_OS_USE_PRIORITY == true
/*! Priority of the task.
 * The lower get the most priority
 */
enum os_priority {
	OS_PRIORITY_1 = 0,
	OS_PRIORITY_2 = 1,
	OS_PRIORITY_3 = 2,
	OS_PRIORITY_4 = 3,
	OS_PRIORITY_5 = 4,
	OS_PRIORITY_10 = 9,
	OS_PRIORITY_20 = 19
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

/*! This structure represents the minimalistic task context
 */
struct os_task_minimal {
	/*! \brief Stack pointer. Will always be the 1rst element of this structure,
	 * to ensure the best optimization.
	 */
	void *sp;
	/*! \brief Pointer of the next task in the list.
	 * Active tasks are registered within a chain list.
	 */
	struct os_task_minimal *next;
#if CONFIG_OS_USE_PRIORITY == true
	/*! \brief Priority of the task.
	 */
	enum os_priority priority;
	/*! \brief Use to manage the task priorities
	 */
	enum os_priority priority_counter;
#endif
};

/*! Structure holding the context of a task
 */
struct os_task {
	/*! \brief Minimal stack context
	 */
	struct os_task_minimal core;
	/*! \brief A pointer on a memory space reserved for the stack
	 */
	uint8_t *stack;
	/*! \brief Task options
	 */
	enum os_task_option options;
};

/*! \brief Task function prototype
 * \param args Arguments passed to the task in a form of an empty pointer
 */
typedef void (*task_ptr_t)(void *args);

/*! \brief This function will define the rules to change the task.
 * \return The new task context
 */
struct os_task_minimal *os_task_scheduler(void);

/*! \defgroup group_os_public_api Public API
 * \brief Public application interface.
 * \ingroup group_os
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
 * os_task_create(&my_task, my_func, NULL, 0, OS_TASK_USE_CUSTOM_STACK);
 * \endcode
 * \ingroup group_os_public_api
 * \param stack_symbol The symbol name used to refer to this stack
 * \param stack_size The size of the stack in bytes
 */
#define OS_MALLOC_STACK(stack_symbol, stack_size) \
		uint8_t (stack_symbol)[(stack_size)]

/*! \brief Convert a delay in milliseconds to a number of ticks.
 * \ingroup group_os_public_api
 * \param time_ms The time (in ms) to be converted
 * \return The number of ticks
 */
#define OS_MS_TO_TICK(time_ms) \
		(((time_ms) * CONFIG_OS_TICK_HZ) / 1000)

/*! \brief Convert a delay in seconds to a number of ticks.
 * \ingroup group_os_public_api
 * \param time_s The time (in s) to be converted
 * \return The number of ticks
 */
#define OS_S_TO_TICK(time_s) \
		((time_s) * CONFIG_OS_TICK_HZ)

#include "os_event.h"

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
bool os_task_create(struct os_task *task, task_ptr_t task_ptr, void *args,
		int stack_size, enum os_task_option options);

/*! \brief Delete a task
 * \ingroup group_os_public_api
 * \param task The task to be deleted
 */
void os_task_delete(struct os_task *task);

/*! \brief Enable the execution a task
 * \ingroup group_os_public_api
 * \param task The task to be enabled
 */
void os_task_enable(struct os_task *task);

/*! \brief Disable the execution of a task
 * \ingroup group_os_public_api
 * \param task The task to be disabled
 */
void os_task_disable(struct os_task *task);

/*! \brief Check wether a task is enabled or not
 * \ingroup group_os_public_api
 * \param task The task to be checked
 * \return true if enabled, false otherwise
 */
bool os_task_is_enabled(struct os_task *task);

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

/*! \brief Get the current running task
 * \ingroup group_os_public_api
 * \return the current task. NULL if none is running.
 */
struct os_task *os_task_current(void);

#if CONFIG_OS_USE_PRIORITY == true
/*! \brief Change the priority of a task
 * \ingroup group_os_public_api
 * \param task The task which needs some update
 * \param priority The new priority
 * \pre \ref CONFIG_OS_USE_PRIORITY needs to be set first
 */
static inline void os_task_set_priority(struct os_task *task, enum os_priority priority) {
	// Not critical so no need to use the os_enter_critical function
	task->core.priority = priority;
	task->core.priority_counter = priority;
}
/*! \brief Get the priority of a task
 * \ingroup group_os_public_api
 * \param task The task which priority is requested
 * \return The task priority
 * \pre \ref CONFIG_OS_USE_PRIORITY needs to be set first
 */
static inline enum os_priority os_task_get_priority(struct os_task *task) {
	return task->core.priority_counter;
}
#endif

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

/* Include OS modules */
#include "os_debug.h"
#include "os_interrupt.h"
#include "os_semaphore.h"

/*! \name Kernel Control
 *
 * Control the core of the operating system
 *
 * \{
 */

/*! \brief Call the scheduler to switch to a new task that is ready to run.
 * This function is useful for cooperative task swiching
 * \ingroup group_os_public_api
 */
void os_yield(void);

/*! \brief Start the task scheduling process
 * \ingroup group_os_public_api
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
	os_yield();
	// Idle loop
	while (true) {
		if (!os_event_scheduler()) {
			HOOK_OS_IDLE();
		}
	}
}

/*! \brief Get the current version of the running operating system
 * \return A string containing the version of the OS.
 */
static inline uint8_t *os_get_version(void) {
	return OS_VERSION;
}
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
 * \fn os_enter_critical
 * \brief Start of a critical code region. Preemptive context switches cannot
 * occur when in a critical region.\n
 */

/*!
 * \fn os_leave_critical
 * \brief Exit a critical code region.\n
 */

/*!
 * \brief Setup the task scheduler interrupt.
 * \param ref_hz The reference frequency used to clock the peripheral which will
 * generate the tick interrupt. Usually this frequency is equal to the CPU
 * frequency.
 */
void os_setup_scheduler(uint32_t ref_hz);

/*!
 * \fn os_task_switch_context(bypass_context_saving)
 * \brief Context switch for a task.\n
 * Function used to schedule and switch between the tasks.\n
 * This function will handle the return from a softwre interrupt. Therefore it
 * can be optimized to bypass the saving context part IF an interrupt is
 * running (if \ref os_interrupt_flag is true).
 * \param bypass_context_saving If true, this function needs to bypass the
 * context saving.
 */

/*!
 * \fn os_task_switch_context_int_handler
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
bool os_task_context_load(struct os_task_minimal *task, task_ptr_t task_ptr,
		void *args);

/*!
 * \brief This function must be called inside the
 * \ref os_task_switch_context_int_handler function in order to switch task
 * context.
 * \return The context of the new task
 */
static inline struct os_task_minimal *os_task_switch_context_int_handler_hook(void) {
#if CONFIG_OS_USE_TICK_COUNTER == true
	extern volatile os_tick_t tick_counter;
	// Update the tick counter
	tick_counter++;
#endif
#if CONFIG_OS_DEBUG
	HOOK_OS_DEBUG_TICK();
#endif
	HOOK_OS_TICK();
	// Task switch context
	return os_task_scheduler();
}

/*!
 * \brief This function must be called inside the
 * \ref os_task_switch_context function in order to switch task context.
 * \return The context of the new task
 */
static inline struct os_task_minimal *os_task_switch_context_hook(void) {
#ifdef OS_SCHEDULER_POST_INTERRUPT_HOOK
	// Clear the software interrupt if needed
	OS_SCHEDULER_POST_INTERRUPT_HOOK();
#endif
#ifdef OS_SCHEDULER_POST_EVENT_HOOK
	// Use the alternate task if any
	OS_SCHEDULER_POST_EVENT_HOOK();
#endif
	// Task switch context
	return os_task_scheduler();
}

/*!
 * \}
 */

/*!
 * \defgroup group_os_internal_api Internal API
 * \ingroup group_os
 * \brief Internal API. These functions should not be used by the user.
 * \{
 */
/*! \brief Test if the current running task is the application task.
 * \return true if the task is the application task, false otherwise.
 */
static inline bool __os_task_is_application(void) {
	extern struct os_task_minimal os_app;
	extern struct os_task_minimal *os_current_task;
	return (os_current_task == &os_app);
}
/*! \brief Get the application task
 * \return the application task
 */
static inline struct os_task_minimal *__os_task_get_application(void) {
	extern struct os_task_minimal os_app;
	return &os_app;
}
/*! \brief Enable the application task
 */
static inline void __os_task_enable_application(void) {
	extern struct os_task_minimal os_app;
	os_task_enable((struct os_task *) &os_app);
}
/*! \brief Disable the application task
 */
static inline void __os_task_disable_application(void) {
	extern struct os_task_minimal os_app;
	os_task_disable((struct os_task *) &os_app);
}
/*! \copydoc os_task_enable
 * This function will push the task at the end of the chain list
 * \warning This function does not check if the task is already added to the
 * list and should be used inside a critical area.
 */
void __os_task_enable(struct os_task_minimal *task);
/*! \copydoc os_task_disable
 * \warning This function does not check if the task is already disabled and
 * should be used inside a critical area. It also does not stop after the
 * execution of this function.
 */
void __os_task_disable(struct os_task_minimal *task);
/*!
 * \}
 */

#endif // __OS_CORE_H__
