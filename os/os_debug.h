/*! \file
 * \brief eeOS Debug
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

#ifndef __OS_DEBUG_H__
#define __OS_DEBUG_H__

/*! \defgroup group_os_debug Debug Mode
 * The debug mode will do the following:
 * - Initially fill the stack with a predefined pattern (\ref CONFIG_OS_DEBUG_UINT8_PATTERN).
 * - Catch stack overflow exceptions.
 */

/*! \def OS_DEBUG_USE_TRACE
 * \ingroup group_os_config
 * \brief Activates the trace functionnality
 * \pre Used when \ref CONFIG_OS_DEBUG is set to true
 */
#ifndef OS_DEBUG_USE_TRACE
	#define OS_DEBUG_USE_TRACE false
#endif

/*! \def CONFIG_OS_DEBUG_UINT8_PATTERN
 * \ingroup group_os_config
 * \brief Defines the default character used to initialy fill the stack
 * \pre Used when \ref CONFIG_OS_DEBUG is set to true
 */
#ifndef CONFIG_OS_DEBUG_UINT8_PATTERN
	#define CONFIG_OS_DEBUG_UINT8_PATTERN 0xaa
#endif

/*! \def HOOK_DEBUG_TRACE
 * \ingroup os_hook
 * \brief Hook called before each trace is logged
 * \param event The event to be traced (value from \ref os_debug_trace_event)
 * \param data Data associated to the event. It depends on the event.
 */
#ifndef HOOK_DEBUG_TRACE
	#define HOOK_DEBUG_TRACE(event, data)
#endif

enum os_debug_trace_event {
	/* OS core related */
	OS_DEBUG_TRACE_CONTEXT_SWITCH = 0x01,
	OS_DEBUG_TRACE_YIELD = 0x02,
	/* Task related */
	OS_DEBUG_TRACE_TASK_CREATE = 0x10,
	OS_DEBUG_TRACE_TASK_SET_PRIORITY = 0x11,
	OS_DEBUG_TRACE_TASK_GET_PRIORITY = 0x12,
	OS_DEBUG_TRACE_TASK_DELETE = 0x13,
	OS_DEBUG_TRACE_TASK_ENABLE = 0x14,
	OS_DEBUG_TRACE_TASK_DISABLE = 0x15,
	OS_DEBUG_TRACE_TASK_DELAY_START = 0x16,
	OS_DEBUG_TRACE_TASK_DELAY_STOP = 0x17,
	/* Software interrupt related */
	OS_DEBUG_TRACE_INTERRUPT_CREATE = 0x20,
	OS_DEBUG_TRACE_INTERRUPT_TRIGGER = 0x21,
	OS_DEBUG_TRACE_INTERRUPT_SET_PRIORITY = 0x22,
	OS_DEBUG_TRACE_INTERRUPT_GET_PRIORITY = 0x23,
	
};

#if OS_DEBUG_USE_TRACE == true
	#define OS_DEBUG_TRACE_LOG(event, data) \
		do { \
			HOOK_DEBUG_TRACE(event, (os_ptr_t) data); \
			os_debug_trace_log(event, (os_ptr_t) data); \
		} while (false)
#else
	#define OS_DEBUG_TRACE_LOG(event, data)
#endif

#define HOOK_OS_DEBUG_APP_INIT() \
		do { \
		} while (false);

#define HOOK_OS_DEBUG_TASK_ADD() \
		do { \
			const uint8_t debug_pattern = CONFIG_OS_DEBUG_UINT8_PATTERN; \
			int i; \
			for (i=0; i<stack_size; i++) { \
				((uint8_t *) task->stack)[i] = debug_pattern; \
			} \
		} while (false);

#define HOOK_OS_DEBUG_TICK() \
		do { \
			struct os_task *current_task = os_task_get_current(); \
			if (current_task != NULL && \
					*((uint8_t *) current_task->stack) != CONFIG_OS_DEBUG_UINT8_PATTERN) { \
				HOOK_OS_STACK_OVERFLOW(); \
				while (true); \
			} \
		} while (false);

struct os_trace {
	os_cy_t time;
	enum os_debug_trace_event event;
	os_ptr_t data;
};

void os_debug_trace_log(enum os_debug_trace_event event, os_ptr_t data);

static inline void os_debug_stop_trace(void) {
	extern bool os_debug_trace_flag;
	/* Disable the trace */
	os_debug_trace_flag = false;
}

void os_debug_start_trace(os_ptr_t buffer, int size);

static inline os_ptr_t os_debug_trace_get_pointer(void) {
	extern struct os_trace *os_debug_trace_ptr;
	return (os_ptr_t) os_debug_trace_ptr;
}

#endif // __OS_DEBUG_H__
