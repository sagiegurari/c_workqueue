#ifndef __WORKQUEUE_H__
#define __WORKQUEUE_H__

#include <stdbool.h>
#include <stddef.h>

#define WORKQUEUE_DEFAULT_QUEUE_SIZE    250

struct WorkQueue;

/**
 * Creates a new work queue of default size.
 */
struct WorkQueue *workqueue_new();

/**
 * Creates a new work queue of provided size.
 * If size = 0, it will use the default size.
 */
struct WorkQueue *workqueue_new_with_options(size_t);

/**
 * Finishes the currently running function and releases all the
 * work queue. All functions in the backlog are dropped and the
 * work queue is no longer usable.
 */
void workqueue_release(struct WorkQueue *);

/**
 * Returns the max queue size.
 */
size_t workqueue_get_queue_size(struct WorkQueue *);

/**
 * Returns the current waiting items count.
 */
size_t workqueue_get_backlog_size(struct WorkQueue *);

/**
 * Adds a new function to the end of the workqueue.
 */
bool workqueue_push(struct WorkQueue *, void (*fn)(void *), void *);

/**
 * Blocks until all queued functions are handled and
 * backlog is empty.
 */
void workqueue_drain(struct WorkQueue *);

#endif

