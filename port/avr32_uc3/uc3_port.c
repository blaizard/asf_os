#include "os_core.h"

/*! Push a 32-bit value onto the stack of the task
 * \param task The current task holding the stack we want to update
 * \param value The 32-bit value to be pushed
 */
#define PUSH(task, value) { \
	(task)->sp = (uint32_t *) (task)->sp - 1; \
	*((uint32_t *) (task)->sp) = (uint32_t) (value); \
}

/* Declaration of the interrupt handler function
 */
#if __GNUC__
__attribute__((__interrupt__)) static void os_task_switch_context_int_handler(void);
#elif __ICCAVR32__
__interrupt static void os_task_switch_context_int_handler(void);
#endif

#if CONFIG_OS_SCHEDULER_TYPE == CONFIG_OS_SCHEDULER_USE_COMPARE
	/* Setup functions to use the compare interrupt
	 */
	void os_setup_scheduler(uint32_t cpu_freq_hz)
	{
		cpu_irq_disable();
		irq_initialize_vectors();
		irq_register_handler((__int_handler) os_task_switch_context_int_handler,
				AVR32_CORE_COMPARE_IRQ, CONFIG_OS_SCHEDULER_IRQ_PRIORITY);
		Set_system_register(AVR32_COMPARE, cpu_freq_hz / CONFIG_OS_TICK_HZ);
		Set_system_register(AVR32_COUNT, 0);
		cpu_irq_enable();
	}

	static inline void os_task_scheduler_clear_int(void) {
		Set_system_register(AVR32_COMPARE, Get_system_register(AVR32_COMPARE));
	}

	#define OS_SCHEDULER_IRQ_GROUP AVR32_CORE_IRQ_GROUP

#elif CONFIG_OS_SCHEDULER_TYPE == CONFIG_OS_SCHEDULER_USE_RTC
	/* Setup functions to use the RTC interrupt
	 */
	#include "rtc.h"

	void os_setup_scheduler(uint32_t pba_freq_hz)
	{
		cpu_irq_disable();
		irq_initialize_vectors();
		irq_register_handler((__int_handler) os_task_switch_context_int_handler,
				AVR32_RTC_IRQ, CONFIG_OS_SCHEDULER_IRQ_PRIORITY);
		// fRTC = 115000 / 4 = 28750
		rtc_init(&AVR32_RTC, RTC_OSC_RC, 1);
		rtc_set_top_value(&AVR32_RTC, 28750 / CONFIG_OS_TICK_HZ);
		rtc_enable_interrupt(&AVR32_RTC);
		rtc_enable(&AVR32_RTC);
		cpu_irq_enable();
	}

	static inline void os_task_scheduler_clear_int(void) {
		(&AVR32_RTC)->icr = AVR32_RTC_ICR_TOPI_MASK;
	}

	#define OS_SCHEDULER_IRQ_GROUP AVR32_RTC_IRQ_GROUP

#elif CONFIG_OS_SCHEDULER_TYPE == CONFIG_OS_SCHEDULER_USE_TC
	/* Setup functions to use the TC interrupt
	 */
	#include "tc.h"

	#define OS_SCHEDULER_TC_IRQ AVR32_TC_IRQ0

	void os_setup_scheduler(uint32_t pba_freq_hz)
	{
		volatile avr32_tc_t *tc = &AVR32_TC;
		tc_waveform_opt_t waveform_opt = {
			.channel = CONFIG_OS_SCHEDULER_TC_CHANNEL,
			.bswtrg = TC_EVT_EFFECT_NOOP,
			.beevt = TC_EVT_EFFECT_NOOP,
			.bcpc = TC_EVT_EFFECT_NOOP,
			.bcpb = TC_EVT_EFFECT_NOOP,
			.aswtrg = TC_EVT_EFFECT_NOOP,
			.aeevt = TC_EVT_EFFECT_NOOP,
			.acpc = TC_EVT_EFFECT_NOOP,
			.acpa = TC_EVT_EFFECT_NOOP,
			.wavsel = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
			.enetrg = false,
			.eevt = 0,
			.eevtedg = TC_SEL_NO_EDGE,
			.cpcdis = false,
			.cpcstop = false,
			.burst = false,
			.clki = false,
			.tcclks = TC_CLOCK_SOURCE_TC3
		};
		tc_interrupt_t tc_interrupt = {
			.etrgs = 0,
			.ldrbs = 0,
			.ldras = 0,
			.cpcs = 1,
			.cpbs = 0,
			.cpas = 0,
			.lovrs = 0,
			.covfs = 0,
		};
		cpu_irq_disable();
		irq_initialize_vectors();
		irq_register_handler((__int_handler) os_task_switch_context_int_handler,
				OS_SCHEDULER_TC_IRQ, CONFIG_OS_SCHEDULER_IRQ_PRIORITY);
		tc_init_waveform(tc, &waveform_opt);
		tc_write_rc(tc, CONFIG_OS_SCHEDULER_TC_CHANNEL,
				(pba_freq_hz + 4 * CONFIG_OS_TICK_HZ) / (8 * CONFIG_OS_TICK_HZ));
		tc_configure_interrupts(tc, CONFIG_OS_SCHEDULER_TC_CHANNEL,
				&tc_interrupt);
		tc_start(tc, CONFIG_OS_SCHEDULER_TC_CHANNEL);
		cpu_irq_enable();
	}

	static inline void os_task_scheduler_clear_int(void) {
		AVR32_TC.channel[CONFIG_OS_SCHEDULER_TC_CHANNEL].sr;
	}

	#define OS_SCHEDULER_IRQ_GROUP AVR32_TC_IRQ_GROUP

#else

	static inline void os_task_scheduler_clear_int(void) {
	}
	#define OS_SCHEDULER_IRQ_GROUP 0
#endif

#if __GNUC__
__attribute__((__naked__))
#elif __ICCAVR32__
	#pragma shadow_registers = full
#endif
ISR(os_task_switch_context_int_handler, OS_SCHEDULER_IRQ_GROUP,
		CONFIG_OS_SCHEDULER_IRQ_PRIORITY)
{
	extern struct os_task_minimal *os_current_task;

	__asm__ __volatile__ (
		// Save context
        	"pushm r0-r7\n\t"

		// Save the stack pointer
		"mov r0, os_current_task\n\t"
		"ld.w r1, r0[0]\n\t"
		"st.w r1[0], sp\n\t"
	);

	// Clear the interrupt flag
	os_task_scheduler_clear_int();
	os_task_switch_context_int_handler_hook();

	__asm__ __volatile__ (
		// Update the stack pointer
		"ld.w sp, r12\n\t"

		// Restore context
		"popm r0-r7\n\t"
		"rete\n\t"
	);
#if __ICCAVR32__
	#pragma diag_suppress=Pe174
#endif
	os_current_task = os_current_task;
#if __ICCAVR32__
	#pragma diag_default=Pe174
#endif
}

#if __GNUC__
__attribute__((__naked__))
void _os_task_switch_context(void);
void _os_task_switch_context(void)
#elif __ICCAVR32__
	#pragma shadow_registers = full
	#pragma exception=0x100,0
__exception void _os_task_switch_context(void)
#endif
{
	extern struct os_task_minimal *os_current_task;

	__asm__ __volatile__ (
		// Save context
		"pushm r10-r12, lr\n\t" // r10-r12, lr
		"ld.d r10, sp[4*4]\n\t"
		"st.d sp[4*4], r8\n\t" // r8-r9
		"st.d --sp, r10\n\t" // pc, sr
        	"pushm r0-r7\n\t" // r0-r7

		// Save the stack pointer
		"mov r0, os_current_task\n\t"
		"ld.w r1, r0[0]\n\t"
		"st.w r1[0], sp\n\t"
	);

	os_task_switch_context_hook();

	__asm__ __volatile__ (
		// Update the stack pointer
		"ld.w sp, r12\n\t"

		// Restore context
		"popm r0-r7\n\t" // r0-r7
		"ld.d r6, sp++\n\t" // r6 = sr; r7 = pc
		"popm lr, r8-r12\n\t" // lr, r8-r12
		"st.d --sp, r6\n\t" // stack sr and pc
		"ld.d r6, sp[-14*4]\n\t" // restore r6-r7

		// Restore context
		"rets\n\t"
	);

#if __ICCAVR32__
	#pragma diag_suppress=Pe174
#endif
	os_current_task = os_current_task;
#if __ICCAVR32__
	#pragma diag_default=Pe174
#endif
}

bool os_task_context_load(struct os_task_minimal *task, task_ptr_t task_ptr, void *args)
{
	PUSH(task, 0); // R8
	PUSH(task, 0); // R9
	PUSH(task, 0); // R10
	PUSH(task, 0); // R11
	PUSH(task, args); // R12
	PUSH(task, 0); // LR
	PUSH(task, (void *) task_ptr); // PC
	PUSH(task, CONFIG_OS_DEFAULT_SR_VALUE);
	PUSH(task, 0); // R0
	PUSH(task, 0); // R1
	PUSH(task, 0); // R2
	PUSH(task, 0); // R3
	PUSH(task, 0); // R4
	PUSH(task, 0); // R5
	PUSH(task, 0); // R6
	PUSH(task, 0); // R7

	return true;
}
