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

#if CONFIG_OS_STATISTICS_MONITOR_TASK_SWITCH == true

/*!
 * Task switching time
 * \{
 */

/*! To compute the number of cycles that a task takes uses
 */
os_cy_t task_cy = 0;
/*! Last context task switching number of cycles
 */
os_cy_t task_switch_cy;
os_cy_t task_switch_min_cy = (uint32_t) -1;
os_cy_t task_switch_max_cy = 0;

static inline void measure_context_task_switch_time_start(os_cy_t current_cy) {
	task_switch_cy = current_cy;
}
static inline void measure_context_task_switch_time_stop(os_cy_t current_cy) {
	task_switch_cy = current_cy - task_switch_cy;
	if (task_switch_cy < task_switch_min_cy) {
		task_switch_min_cy = task_switch_cy;
	}
	if (task_switch_cy > task_switch_max_cy) {
		task_switch_max_cy = task_switch_cy;
	}
}

static inline void update_task_cycle_counter_start(os_cy_t current_cy) {
	struct os_process *current_process = __os_process_get_current();
	current_process->cycle_counter += current_cy - task_cy;
}
static inline void update_task_cycle_counter_stop(os_cy_t current_cy) {
	task_cy = current_cy;
}

void __os_statistics_switch_context_tick_handler_start(os_cy_t offset_cy)
{
	os_cy_t current_cy = os_read_cycle_counter() - offset_cy;
	measure_context_task_switch_time_start(current_cy);
	update_task_cycle_counter_start(current_cy);
}
void __os_statistics_switch_context_tick_handler_stop(os_cy_t offset_cy)
{
	os_cy_t current_cy = os_read_cycle_counter() + offset_cy;
	measure_context_task_switch_time_stop(current_cy);
	update_task_cycle_counter_stop(current_cy);
}

void __os_statistics_switch_context_start(os_cy_t offset_cy)
{
	os_cy_t current_cy = os_read_cycle_counter() - offset_cy;
	update_task_cycle_counter_start(current_cy);
}
void __os_statistics_switch_context_stop(os_cy_t offset_cy)
{
	os_cy_t current_cy = os_read_cycle_counter() + offset_cy;
	update_task_cycle_counter_stop(current_cy);
}

/*! \copybrief os_statistics_get_task_switch_time_jitter
 */
os_cy_t os_statistics_get_task_switch_time_jitter(void)
{
	return (task_switch_max_cy - task_switch_min_cy) / 2;
}

/*! \copybrief os_statistics_get_task_switch_time
 */
os_cy_t os_statistics_get_task_switch_time(void)
{
	return (task_switch_max_cy + task_switch_min_cy) / 2;
}

/*!
 * \}
 */

#endif // CONFIG_OS_STATISTICS_MONITOR_TASK_SWITCH == true

/*!
 * The ratio of the CPU ressources is calculated as follow:
 * \code ratio = (100 / (priority level)) / SUM(100 / (each priority level)) \endcode
 */
uint8_t os_statistics_task_cpu_allocation(struct os_task *task)
{
	struct os_process *proc = __os_task_get_process(task);
	struct os_process *last_proc = proc->next;
#if CONFIG_OS_USE_PRIORITY == true
	uint8_t priority = 100 / __os_process_get_priority(proc);
#else
	uint8_t priority = 100;
#endif
	uint16_t sum = priority;

	// Loop into the task list
	while (last_proc != proc) {
		if (__os_process_is_task(last_proc)) {
#if CONFIG_OS_USE_PRIORITY == true
			sum += 100 / __os_process_get_priority(last_proc);
#else
			sum += 100;
#endif
		}
		last_proc = last_proc->next;
	}

	return (uint8_t) (((uint16_t) priority * 100) / sum);
}

#endif // CONFIG_OS_USE_STATISTICS == true
