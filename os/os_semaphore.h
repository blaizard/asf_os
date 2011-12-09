#ifndef __OS_SEMAPHORE_H__
#define __OS_SEMAPHORE_H__

struct os_semaphore {
	uint16_t counter;
	uint16_t max;
};

/*! \brief Creates a counting semaphore
 * \param sem The un-initialized sempahore structure
 * \param counter The maximum count value that can be reached
 * \param initial_count The count value assigned to the semaphore when it is
 * created
 */
static inline void os_semaphore_create(struct os_semaphore *sem, uint16_t counter,
		uint16_t initial_count) {
	sem->counter = initial_count;
	sem->max = counter;
}

/*! \brief Creates a binary semaphore
 * \param sem The un-initialized sempahore structure
 */
static inline void os_binary_semaphore_create(struct os_semaphore *sem) {
	os_semaphore_create(sem, 1, 1);
}

/*! \brief Creates an event from a semaphore. The sempahore must have been
 * previously created before using this function
 * \param event The un-initialized event structure
 * \param sem The semaphore which will be linked to this event
 */
static inline void os_semaphore_create_event(struct os_event *event,
		struct os_semaphore *sem) {
	extern const struct os_event_descriptor semaphore_event_descriptor;
	os_event_create(event, &semaphore_event_descriptor, (void *) sem);
}

/*! \brief Take a semaphore. If no semaphore is available, wait until it gets
 * free
 * \param sem The semaphore to take
 * \pre The semaphore must have previously been created
 */
void os_semaphore_take(struct os_semaphore *sem);

/*! \brief Releases a semaphore.
 * \param sem The semaphore to release
 * \pre The semaphore must have previously been created
 */
void os_sempahore_release(struct os_semaphore *sem);

#endif // __OS_SEMAPHORE_H__
