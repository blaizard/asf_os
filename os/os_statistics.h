/*! \file
 * \brief eeOS Statistics
 * \author Blaise Lengrand (blaise.lengrand@gmail.com)
 * \version 0.1
 * \date 2011
 *
 * \section eeos_license License
 * \ref eeos is provided in source form for FREE evaluation, for
 * educational use or for peaceful research. If you plan on using \ref eeos in a
 * commercial product you need to contact the author to properly license
 * its use in your product. The fact that the  source is provided does
 * NOT mean that you can use it without paying a licensing fee.
 */

#ifndef __OS_STATISTICS_H__
#define __OS_STATISTICS_H__


#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_START(offset_cy) \
	do { \
		os_statistics_switch_context_start(offset_cy); \
	} while (false)

#define HOOK_OS_STATISTICS_SWITCH_CONTEXT_STOP() \
	do { \
		os_statistics_switch_context_stop(); \
	} while (false)

void os_statistics_switch_context_start(os_cy_t offset_cy);
void os_statistics_switch_context_stop(void);

os_cy_t os_statistics_get_task_switch(void);
os_cy_t os_statistics_get_task_switch_jitter(void);

void os_statistics_monitor_ram(void);

#endif // __OS_STATISTICS_H__
