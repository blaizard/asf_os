#include "os_core.h"

#if CONFIG_OS_USE_SW_INTERRUPTS == true

bool os_interrupt_flag = false;

void __os_interrupt_handler(void *args)
{
	struct os_interrupt *interrupt = (struct os_interrupt *) args;

	// Disable scheduler interrupts
	os_enter_critical();
	// Remove the interrupt from the chain list to prevent another execution
	__os_task_disable((struct os_task_minimal *) interrupt);
	// Execute the interrupt handler
	interrupt->task_ptr(interrupt->args);
	// Manually call the scheduler
	os_task_switch_context(true);
}

void os_interrupt_setup(struct os_interrupt *interrupt, task_ptr_t task_ptr,
		void *args)
{
	// Fill the structure
	interrupt->task_ptr = task_ptr;
	interrupt->args = args;
	// Setup the task to run in the application context
	interrupt->core.sp = NULL;
#if CONFIG_OS_USE_PRIORITY == true
	os_interrupt_set_priority(interrupt, CONFIG_OS_INTERRUPT_DEFAULT_PRIORITY);
#endif
}

#endif
