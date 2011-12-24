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

#ifndef __OS_QUEUE_H__
#define __OS_QUEUE_H__

/*! \name Process queue helper function set
 *
 * \{
 */

struct os_queue {
	struct os_queue *next;
};

typedef bool (*os_queue_sort_t)(struct os_queue *a, struct os_queue *b);

/*! \brief Helper function used to define the order of a new elements added to a
 * queue (\ref os_queue). This function will add them so that the \b first \b in
 * will be the \b first \b out. This function must be used with
 * \ref os_event_descriptor::sort.
 */
bool os_queue_sort_fifo(struct os_queue *a, struct os_queue *b);
/*! \brief Helper function used to define the order of a new elements added to a
 * queue (\ref os_queue). This function will add them so that the \b last \b in
 * will be the \b first \b out. This function must be used with
 * \ref os_event_descriptor::sort.
 */
bool os_queue_sort_lifo(struct os_queue *a, struct os_queue *b);

#if CONFIG_OS_USE_PRIORITY == true
/*! \brief Helper function used to define the order of a new elements added to a
 * queue (\ref os_queue). This function will add them so that the highest
 * priority process will be at the head of the list. This function must be used
 * with \ref os_event_descriptor::sort.
 * \pre \ref CONFIG_OS_USE_PRIORITY must be set
 */
bool os_queue_process_sort_priority(struct os_queue *a, struct os_queue *b);
#endif

static inline struct os_queue *os_queue_pop(struct os_queue **first_elt) {
	struct os_queue *elt = *first_elt;
	*first_elt = elt->next;
	return elt;
}

static inline struct os_queue *os_queue_head(struct os_queue *first_elt) {
	return first_elt;
}

static inline void os_queue_insert_after(struct os_queue *prev_elt,
		struct os_queue *elt) {
	elt->next = prev_elt->next;
	prev_elt->next = elt;
}

static inline void os_queue_insert_first(struct os_queue **first_elt,
		struct os_queue *elt) {
	elt->next = *first_elt;
	*first_elt = elt;
}

void os_queue_add_sort(struct os_queue **first_elt,
		struct os_queue *elt, os_queue_sort_t sort_fct);

static inline void os_queue_add(struct os_queue **first_elt,
		struct os_queue *new_elt) {
	os_queue_add_sort(first_elt, new_elt, os_queue_sort_fifo);
}

bool os_queue_remove(struct os_queue **first_elt, struct os_queue *elt);
/*!
 * \}
 */


struct os_queue_bidirectional {
	struct os_queue_bidirectional *next;
	struct os_queue_bidirectional *prev;
};

typedef bool (*os_queue_bidirectional_sort_t)(struct os_queue_bidirectional *a, struct os_queue_bidirectional *b);

#if CONFIG_OS_USE_PRIORITY == true
/*! \brief Helper function used to define the order of a new elements added to a
 * queue (\ref os_queue). This function will add them so that the highest
 * priority process will be at the head of the list. This function must be used
 * with \ref os_event_descriptor::sort.
 * \pre \ref CONFIG_OS_USE_PRIORITY must be set
 */
bool os_queue_bidirectional_process_sort_priority(
		struct os_queue_bidirectional *a,
		struct os_queue_bidirectional *b);

#endif

static inline struct os_queue_bidirectional *os_queue_bidirectional_pop(
		 struct os_queue_bidirectional **first_elt) {
	struct os_queue_bidirectional *elt = *first_elt;
	*first_elt = elt->next;
	(*first_elt)->prev = NULL;
	return elt;
}

static inline struct os_queue_bidirectional *os_queue_bidirectional_head(
		struct os_queue_bidirectional *first_elt) {
	return first_elt;
}

static inline void os_queue_bidirectional_insert_after(
		struct os_queue_bidirectional *prev_elt,
		struct os_queue_bidirectional *elt) {
	struct os_queue_bidirectional *next_elt = prev_elt->next;
	if (next_elt) {
		next_elt->prev = elt;
	}
	elt->next = next_elt;
	elt->prev = prev_elt;
	prev_elt->next = elt;
}

static inline void os_queue_bidirectional_insert_first(
		struct os_queue_bidirectional **first_elt,
		struct os_queue_bidirectional *elt) {
	elt->next = *first_elt;
	(*first_elt)->prev = elt;
	*first_elt = elt;
}

void os_queue_bidirectional_add_sort(struct os_queue_bidirectional **first_elt,
		struct os_queue_bidirectional *elt,
		os_queue_bidirectional_sort_t sort_fct);

static inline void os_queue_bidirectional_add(
		struct os_queue_bidirectional **first_elt,
		struct os_queue_bidirectional *new_elt) {
	os_queue_bidirectional_add_sort(first_elt, new_elt,
			(os_queue_bidirectional_sort_t) os_queue_sort_fifo);
}

void os_queue_bidirectional_remove(struct os_queue_bidirectional *elt);

void os_queue_bidirectional_remove_ex(struct os_queue_bidirectional **first_elt,
		struct os_queue_bidirectional *elt);


#define OS_QUEUE_DEFINE(NAME, ...) \
	struct os_queue_##NAME { \
		struct os_queue_##NAME *next; \
		__VA_ARGS__ \
	}; \
	static inline struct os_queue_##NAME *os_queue_##NAME##_pop(struct os_queue_##NAME **first_elt) { \
		return (struct os_queue_##NAME *) os_queue_pop((struct os_queue **) first_elt); \
	} \
	static inline struct os_queue_##NAME *os_queue_##NAME##_head(struct os_queue_##NAME *first_elt) { \
		return (struct os_queue_##NAME *) os_queue_head((struct os_queue *) first_elt); \
	} \
	static inline void os_queue_##NAME##_insert_after(struct os_queue_##NAME *prev_elt, struct os_queue_##NAME *elt) { \
		os_queue_insert_after((struct os_queue *) prev_elt, (struct os_queue *) elt); \
	} \
	static inline void os_queue_##NAME##_insert_first(struct os_queue_##NAME **first_elt, struct os_queue_##NAME *elt) { \
		os_queue_insert_first((struct os_queue **) first_elt, (struct os_queue *) elt); \
	} \
	static inline void os_queue_##NAME##_add_sort(struct os_queue_##NAME **first_elt, struct os_queue_##NAME *elt, os_queue_sort_t sort_fct) { \
		os_queue_add_sort((struct os_queue **) first_elt, (struct os_queue *) elt, sort_fct); \
	} \
	static inline bool os_queue_##NAME##_remove(struct os_queue_##NAME **first_elt, struct os_queue_##NAME *elt) { \
		return os_queue_remove((struct os_queue **) first_elt, (struct os_queue *) elt); \
	}

#define OS_QUEUE_BIDIRECTIONAL_DEFINE(NAME, ...) \
	struct os_queue_##NAME { \
		struct os_queue_##NAME *next; \
		struct os_queue_##NAME *prev; \
		__VA_ARGS__ \
	}; \
	static inline struct os_queue_##NAME *os_queue_##NAME##_pop(struct os_queue_##NAME **first_elt) { \
		return (struct os_queue_##NAME *) os_queue_bidirectional_pop((struct os_queue_bidirectional **) first_elt); \
	} \
	static inline struct os_queue_##NAME *os_queue_##NAME##_head(struct os_queue_##NAME *first_elt) { \
		return (struct os_queue_##NAME *) os_queue_bidirectional_head((struct os_queue_bidirectional *) first_elt); \
	} \
	static inline void os_queue_##NAME##_insert_after(struct os_queue_##NAME *prev_elt, struct os_queue_##NAME *elt) { \
		os_queue_bidirectional_insert_after((struct os_queue_bidirectional *) prev_elt, (struct os_queue_bidirectional *) elt); \
	} \
	static inline void os_queue_##NAME##_insert_first(struct os_queue_##NAME **first_elt, struct os_queue_##NAME *elt) { \
		os_queue_bidirectional_insert_first((struct os_queue_bidirectional **) first_elt, (struct os_queue_bidirectional *) elt); \
	} \
	static inline void os_queue_##NAME##_add_sort(struct os_queue_##NAME **first_elt, struct os_queue_##NAME *elt, os_queue_bidirectional_sort_t sort_fct) { \
		os_queue_bidirectional_add_sort((struct os_queue_bidirectional **) first_elt, (struct os_queue_bidirectional *) elt, sort_fct); \
	} \
	static inline void os_queue_##NAME##_remove(struct os_queue_##NAME *elt) { \
		os_queue_bidirectional_remove((struct os_queue_bidirectional *) elt); \
	} \
	static inline void os_queue_##NAME##_remove_ex(struct os_queue_##NAME **first_elt, struct os_queue_##NAME *elt) { \
		os_queue_bidirectional_remove_ex((struct os_queue_bidirectional **) first_elt, (struct os_queue_bidirectional *) elt); \
	}

OS_QUEUE_DEFINE(process,
	struct os_process *proc;
)

#if CONFIG_OS_USE_PRIORITY == true
static inline void os_queue_process_add(struct os_queue_process **first_elt,
		struct os_queue_process *elt) {
	os_queue_add_sort((struct os_queue **) first_elt,(struct os_queue *) elt, os_queue_process_sort_priority);
}
#else
static inline void os_queue_process_add(struct os_queue_process **first_elt,
		struct os_queue_process *elt) {
	os_queue_add_sort((struct os_queue **) first_elt,(struct os_queue *) elt, os_queue_sort_fifo);
}
#endif

OS_QUEUE_BIDIRECTIONAL_DEFINE(bidirectional_process,
	struct os_process *proc;
)

#if CONFIG_OS_USE_PRIORITY == true
static inline void os_queue_bidirectional_process_add(struct os_queue_bidirectional_process **first_elt,
		struct os_queue_bidirectional_process *elt) {
	os_queue_bidirectional_add_sort((struct os_queue_bidirectional **) first_elt,
			(struct os_queue_bidirectional *) elt,
			os_queue_bidirectional_process_sort_priority);
}
#else
static inline void os_queue_bidirectional_process_add(struct os_queue_bidirectional_process **first_elt,
		struct os_queue_bidirectional_process *elt) {
	os_queue_bidirectional_add_sort((struct os_queue_bidirectional **) first_elt,
			(struct os_queue_bidirectional *) elt, (os_queue_bidirectional_sort_t) os_queue_sort_fifo);
}
#endif

#endif // __OS_QUEUE_H__
