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

#include "os_core.h"

#if CONFIG_OS_USE_STATISTICS == true

#if CONFIG_OS_STATISTICS_TASK_SWITCHING == true

/*!
 * Task switching time
 * \{
 */

os_cy_t task_switch_cy;
os_cy_t task_switch_min_cy = (uint32_t) -1;
os_cy_t task_switch_max_cy = 0;

void __os_statistics_switch_context_start(os_cy_t offset_cy)
{
	task_switch_cy = os_read_cycle_counter() - offset_cy;
}

void __os_statistics_switch_context_stop(void)
{
	task_switch_cy = os_read_cycle_counter() - task_switch_cy;
	if (task_switch_cy < task_switch_min_cy) {
		task_switch_min_cy = task_switch_cy;
	}
	if (task_switch_cy > task_switch_max_cy) {
		task_switch_max_cy = task_switch_cy;
	}
}

os_cy_t os_statistics_get_task_switch_time_jitter(void)
{
	return (task_switch_max_cy - task_switch_min_cy) / 2;
}

os_cy_t os_statistics_get_task_switch_time(void)
{
	return (task_switch_max_cy + task_switch_min_cy) / 2;
}

/*!
 * \}
 */

#endif // CONFIG_OS_STATISTICS_TASK_SWITCHING == true

#endif // CONFIG_OS_USE_STATISTICS == true
