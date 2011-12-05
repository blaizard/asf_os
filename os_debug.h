#ifndef __OS_DEBUG_H__
#define __OS_DEBUG_H__

/*! \defgroup group_os_debug Debug Mode
 * The debug mode will do the following:
 * - Initially fill the stack with a predefined pattern (\ref CONFIG_OS_DEBUG_CHAR_PATTERN).
 * - Catch stack overflow exceptions.
 */

/*! \def CONFIG_OS_DEBUG_CHAR_PATTERN
 * \ingroup group_os_config
 * \brief Defines the default character used to initialy fill the stack
 * \pre Used when \ref CONFIG_OS_DEBUG is set to true
 */
#ifndef CONFIG_OS_DEBUG_CHAR_PATTERN
	#define CONFIG_OS_DEBUG_CHAR_PATTERN 0xaa
#endif

#define HOOK_OS_DEBUG_TASK_ADD() \
		do { \
			const uint8_t debug_pattern = CONFIG_OS_DEBUG_CHAR_PATTERN; \
			int i; \
			for (i=0; i<stack_size; i++) { \
				task->stack[i] = debug_pattern; \
			} \
		} while (false);

#define HOOK_OS_DEBUG_TICK() \
		do { \
			struct os_task *current_task = os_task_current(); \
			if (*current_task->stack != CONFIG_OS_DEBUG_CHAR_PATTERN \
					&& current_task) { \
				HOOK_OS_STACK_OVERFLOW(); \
				while (true); \
			} \
		} while (false);

#endif // __OS_DEBUG_H__
