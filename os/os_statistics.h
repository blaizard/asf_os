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

#if CONFIG_OS_USE_STATISTICS == true
	#if CONFIG_OS_STATISTICS_MONITOR_TASK_SWITCH == true
		void __os_statistics_switch_context_tick_handler_start(os_cy_t offset_cy);
		void __os_statistics_switch_context_tick_handler_stop(os_cy_t offset_cy);
		void __os_statistics_switch_context_start(os_cy_t offset_cy);
		void __os_statistics_switch_context_stop(os_cy_t offset_cy);
		/*! \brief Hook use to get a time information at the entrance of
		 * a context task switch.
		 * \param offset_cy The number of cycles before the call of
		 * \ref HOOK_OS_STATISTICS_SWITCH_CONTEXT_START and after the
		 * call of \ref HOOK_OS_STATISTICS_SWITCH_CONTEXT_STOP within
		 * the context task switch operation.
		 */
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_TICK_HANDLER_START(offset_cy) \
			do { \
				__os_statistics_switch_context_tick_handler_start(offset_cy); \
			} while (false)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_TICK_HANDLER_STOP(offset_cy) \
			do { \
				__os_statistics_switch_context_tick_handler_stop(offset_cy); \
			} while (false)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_START(offset_cy) \
			do { \
				__os_statistics_switch_context_start(offset_cy); \
			} while (false)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_STOP(offset_cy) \
			do { \
				__os_statistics_switch_context_stop(offset_cy); \
			} while (false)
	#else
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_TICK_HANDLER_START(offset_cy)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_TICK_HANDLER_STOP(offset_cy)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_START(offset_cy)
		#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_STOP(offset_cy)
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
 * \pre \ref CONFIG_OS_STATISTICS_MONITOR_TASK_SWITCH must be set
 */
os_cy_t os_statistics_get_task_switch_time(void);

/*!
 * \brief Get the jitter of the context task switching in number of cycles
 * \ingroup group_os_public_api
 * \return the number of cycles of the switching time jitter
 * \pre \ref CONFIG_OS_STATISTICS_MONITOR_TASK_SWITCH must be set
 */
os_cy_t os_statistics_get_task_switch_time_jitter(void);

/*!
 * \brief Theoretical estimation of the CPU load of a task.
 * It is based on the number of current active tasks in the list and their
 * priority.
 * \param task The task to evaluate
 * \return The allocation time in percent of the CPU assigned to this task
 */
uint8_t os_statistics_task_cpu_allocation(struct os_task *task);

/*!
 * \}
 */

void os_statistics_monitor_ram(void);

#endif // __OS_STATISTICS_H__
