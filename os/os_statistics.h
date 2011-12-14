/*! \file
 * \brief eeOS Statistics
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

#ifndef __OS_STATISTICS_H__
#define __OS_STATISTICS_H__


/*! \ingroup group_os_config
 * \pre \ref CONFIG_OS_STATISTICS_TASK_SWITCHING must be set
 *
 * \{
 */

/*! \def CONFIG_OS_STATISTICS_TASK_SWITCHING
 * \brief Give statistics about the task switching.
 * This enables the following functions:
 * - \ref os_statistics_get_task_switch_time
 * - \ref os_statistics_get_task_switch_time_jitter
 */
#ifndef CONFIG_OS_STATISTICS_TASK_SWITCHING
	#define CONFIG_OS_STATISTICS_TASK_SWITCHING true
#endif

/*!
 * \}
 */

#if CONFIG_OS_USE_STATISTICS == true
	#if CONFIG_OS_STATISTICS_TASK_SWITCHING == true
		void __os_statistics_switch_context_start(os_cy_t offset_cy);
		void __os_statistics_switch_context_stop(void);
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_START(offset_cy) \
			do { \
				__os_statistics_switch_context_start(offset_cy); \
			} while (false)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_STOP() \
			do { \
				__os_statistics_switch_context_stop(); \
			} while (false)
	#else
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_START(offset_cy)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_STOP()
	#endif
#endif

/*! \name Statistics
 *
 * This set of function gives real-time statistics on the current operating
 * system setup.
 *
 * \pre \ref CONFIG_OS_USE_STATISTICS needs to be set
 *
 * \ingroup group_os_public_api
 *
 * \{
 */

/*!
 * \brief Get the average time of the context task switching in number of cycles
 * \ingroup group_os_public_api
 * \return the number of cycles of the average context switching time
 * \pre \ref CONFIG_OS_STATISTICS_TASK_SWITCHING must be set
 */
os_cy_t os_statistics_get_task_switch_time(void);

/*!
 * \brief Get the jitter of the context task switching in number of cycles
 * \ingroup group_os_public_api
 * \return the number of cycles of the switching time jitter
 * \pre \ref CONFIG_OS_STATISTICS_TASK_SWITCHING must be set
 */
os_cy_t os_statistics_get_task_switch_time_jitter(void);

/*!
 * \}
 */

void os_statistics_monitor_ram(void);

#endif // __OS_STATISTICS_H__
