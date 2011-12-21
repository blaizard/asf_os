/*! \file
 * \brief eeOS Queues
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

bool os_queue_sort_fifo(struct os_queue *a, struct os_queue *b)
{
	return true;
}

bool os_queue_sort_lifo(struct os_queue *a, struct os_queue *b)
{
	return false;
}

#if CONFIG_OS_USE_PRIORITY == true
bool os_queue_process_sort_priority(struct os_queue *a, struct os_queue *b)
{
	struct os_process *proc1 = ((struct os_queue_process *) a)->proc;
	struct os_process *proc2 = ((struct os_queue_process *) b)->proc;
	return (proc1->priority <= proc2->priority);
}
#endif

void os_queue_add_sort(struct os_queue **first_elt,
		struct os_queue *new_elt,
		bool (*sort_fct)(struct os_queue *, struct os_queue *))
{
	struct os_queue *current_elt;
	struct os_queue *prev_elt = NULL;

	/* Add the process to the list */
	current_elt = *first_elt;
	while (current_elt && sort_fct(current_elt, new_elt)) {
		prev_elt = current_elt;
		current_elt = current_elt->next;
	}
	// If the process is supposed to be at the beginning of the list
	if (prev_elt) {
		os_queue_insert_after(prev_elt, new_elt);
	}
	else {
		os_queue_insert_first(first_elt, new_elt);
	}
}

bool os_queue_remove(struct os_queue **first_elt, struct os_queue *elt)
{
	struct os_queue *current_elt = *first_elt;
	struct os_queue *prev_elt = NULL;

	while (current_elt) {
		if (current_elt == elt) {
			if (prev_elt) {
				prev_elt->next = current_elt->next;
			}
			else {
				*first_elt = current_elt->next;
			}
			return true;
		}
		prev_elt = current_elt;
		current_elt = current_elt->next;
	}
	return false;
}
