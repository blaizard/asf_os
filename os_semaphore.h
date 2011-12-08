#ifndef __OS_SEMAPHORE_H__
#define __OS_SEMAPHORE_H__

struct os_semaphore {
	struct os_event e;
	uint16_t counter;
};
/*
void os_semaphore_create(struct os_semaphore *sem, uint16_t counter) {
	sem->e.is_triggered = os_sempahore_is_triggered;
	sem->counter = counter;
}

void os_sempahore_is_triggered(struct os_semaphore *sem) {
	
}

void os_semaphore_take(struct os_semaphore) {
	// Take if we can
	os_enter_critical();
	if (sem->counter > 0)
		sem->counter--;
	os_leave_critical();

	while () {
		os_yield();
	}
}

void os_semaphore_take
*/
#endif // __OS_SEMAPHORE_H__
