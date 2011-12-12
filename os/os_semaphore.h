/*! \file
 * \brief eeOS Semaphores
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

#ifndef __OS_SEMAPHORE_H__
#define __OS_SEMAPHORE_H__

/*! \brief Semaphore Structure
 */
struct os_semaphore {
	/*! \brief Counter to hold the current number of free semaphores
	 */
	uint16_t counter;
	/*! \brief Maximum semaphore available
	 */
	uint16_t max;
};

/*! \name Semaphores
 *
 * Set of functions to create and control sempahores
 *
 * \{
 */

/*! \brief Creates a counting semaphore
 * \ingroup group_os_public_api
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
 * \ingroup group_os_public_api
 * \param sem The un-initialized sempahore structure
 */
static inline void os_binary_semaphore_create(struct os_semaphore *sem) {
	os_semaphore_create(sem, 1, 1);
}

/*! \brief Creates an event from a semaphore. The sempahore must have been
 * previously created before using this function
 * \ingroup group_os_public_api
 * \param event The un-initialized event structure
 * \param sem The semaphore which will be linked to this event
 * \pre \ref CONFIG_OS_USE_EVENTS must be set
 */
static inline void os_semaphore_create_event(struct os_event *event,
		struct os_semaphore *sem) {
	extern const struct os_event_descriptor semaphore_event_descriptor;
	os_event_create(event, &semaphore_event_descriptor, (void *) sem);
}

/*! \brief Take a semaphore. If no semaphore is available, wait until it gets
 * free
 * \ingroup group_os_public_api
 * \param sem The semaphore to take
 * \pre The semaphore must have previously been created
 */
void os_semaphore_take(struct os_semaphore *sem);

/*! \brief Releases a semaphore.
 * \ingroup group_os_public_api
 * \param sem The semaphore to release
 * \pre The semaphore must have previously been created
 */
void os_sempahore_release(struct os_semaphore *sem);

/*!
 * \}
 */

#endif // __OS_SEMAPHORE_H__
