#ifndef __OS_PORT_H__
#define __OS_PORT_H__

#include "interrupt.h"

/*!
 * \ingroup os_scheduler_type
 * \brief Use the RTC peripheral to generate the scheduler interrupts
 */
#define CONFIG_OS_SCHEDULER_USE_RTC 1
/*!
 * \ingroup os_scheduler_type
 * \brief Use the internal compare feature to generate the scheduler interrupts.
 * Using this configuration, will affect the internal cycle counter value.
 */
#define CONFIG_OS_SCHEDULER_USE_COMPARE 2
/*!
 * \ingroup os_scheduler_type
 * \brief Use the TC peripheral to generate the scheduler interrupts
 */
#define CONFIG_OS_SCHEDULER_USE_TC 3

/*! \def CONFIG_OS_DEFAULT_SR_VALUE
 * \ingroup group_os_config
 * \brief This is the default value assigned to the Status Register (SR) before
 * running a new task
 */
#ifndef CONFIG_OS_DEFAULT_SR_VALUE
	//! SR: [M2:M0]=001 I1M=0 I0M=0, GM=0
	#define CONFIG_OS_DEFAULT_SR_VALUE 0x00400000
#endif

/*! \def CONFIG_OS_SCHEDULER_IRQ_PRIORITY
 * \ingroup group_os_config
 * \brief IRQ priority of the tick interrupt
 */
#ifndef CONFIG_OS_SCHEDULER_IRQ_PRIORITY
	#define CONFIG_OS_SCHEDULER_IRQ_PRIORITY 0
#endif

/*! \def CONFIG_OS_SCHEDULER_TC_CHANNEL
 * \ingroup group_os_config
 * \brief TC channel used to generate the tick interrupt
 * \pre \ref CONFIG_OS_SCHEDULER_USE_TC must be used
 */
#ifndef CONFIG_OS_SCHEDULER_TC_CHANNEL
	#define CONFIG_OS_SCHEDULER_TC_CHANNEL 0
#endif

static inline void os_enter_critical(void) {
	cpu_irq_disable();
};

static inline void os_leave_critical(void) {
	cpu_irq_enable();
};

#define os_task_switch_context(bypass_context_saving) \
	do { \
		__asm__ __volatile__ ( \
			"mov r8, "ASTRINGZ(bypass_context_saving)"\n\t" \
			"scall\n\t" \
		); \
	} while (false)

#endif // __OS_PORT_H__
