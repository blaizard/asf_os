/*! \file
 * \brief eeOS Mutex
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

#ifndef __OS_MUTEX_H__
#define __OS_MUTEX_H__

/*! \brief Mutex Structure
 */
struct os_mutex {
	/*! \brief Defines if a mutex is locked or not
	 */
	bool is_locked;
	/*! \brief The process which locked the mutex
	 */
	struct os_process *process;
};

/*! \name Mutex
 *
 * Set of functions to create and control mutex
 *
 * \{
 */

/*! \brief Creates a mutex
 * \ingroup group_os_public_api
 * \param mutex The un-initialized mutex structure
 */
static inline void os_mutex_create(struct os_mutex *mutex) {
	mutex->is_locked = false;
}

/*! \brief Creates an event from a mutex. The mutex must have been
 * previously created before using this function
 * \ingroup group_os_public_api
 * \param event The un-initialized event structure
 * \param mutex The mutex which will be linked to this event
 * \pre \ref CONFIG_OS_USE_EVENTS must be set
 */
static inline void os_mutex_create_event(struct os_event *event,
		struct os_mutex *mutex) {
	extern const struct os_event_descriptor mutex_event_descriptor;
	os_event_create(event, &mutex_event_descriptor, (os_ptr_t) mutex);
}

/*! \brief Lock a mutex. If the mutex is already locked, wait until it gets
 * unlocked
 * \ingroup group_os_public_api
 * \param mutex The mutex to be locked
 * \pre The mutex must have previously been created
 */
void os_mutex_lock(struct os_mutex *mutex);

/*! \brief Un-lock a mutex.
 * \ingroup group_os_public_api
 * \param mutex The mutex to be unlocked
 * \pre The mutex must have previously been created
 */
void os_mutex_unlock(struct os_mutex *mutex);

/*!
 * \}
 */

#endif // __OS_MUTEX_H__