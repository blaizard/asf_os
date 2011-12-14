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

/*! \def CONFIG_OS_DEBUG_UINT8_PATTERN
 * \ingroup group_os_config
 * \brief Defines the default character used to initialy fill the stack
 * \pre Used when \ref CONFIG_OS_DEBUG is set to true
 */
#ifndef CONFIG_OS_DEBUG_UINT8_PATTERN
	#define CONFIG_OS_DEBUG_UINT8_PATTERN 0xaa
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
			struct os_task *current_task = os_task_current(); \
			if (current_task != NULL && \
					*((uint8_t *) current_task->stack) != CONFIG_OS_DEBUG_UINT8_PATTERN) { \
				HOOK_OS_STACK_OVERFLOW(); \
				while (true); \
			} \
		} while (false);

#endif // __OS_DEBUG_H__
