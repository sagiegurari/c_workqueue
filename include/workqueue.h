#ifndef __WORKQUEUE_H__
#define __WORKQUEUE_H__

#include <stdbool.h>
#include <stddef.h>

#define WORKQUEUE_DEFAULT_QUEUE_SIZE    250

struct WorkQueue;

/**
 * Custom threading API used by the workqueue and provided
 * by the caller.
 * The API will only require to handle a single thread at a time.
 */
struct WorkQueueThreadAPI
{
  /**
   * Creates and starts a new thread.
   * A basic implementation can be to simply wrap the pthread_create call.
   */
  bool (*start)(struct WorkQueueThreadAPI *, void *(*fn)(void *), void *args);
  /**
   * Stops the currently running thread (if exists).
   * A basic implementation can be to simply wrap the pthread_join call.
   */
  void (*stop)(struct WorkQueueThreadAPI *);
  /**
   * Any custom context used by the caller (not used by workqueue).
   */
  void *context;
};

/**
 * Creates a new work queue of default size.
 */
struct WorkQueue *workqueue_new(void);

/**
 * Creates a new work queue of provided size.
 * If size = 0, it will use the default size.
 * The optional thread api can be provided to gain more control over thread creation.
 */
struct WorkQueue *workqueue_new_with_options(size_t, struct WorkQueueThreadAPI *);

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
 * Returns true if a function is now being invoked by the workqueue.
 */
bool workqueue_is_busy(struct WorkQueue *);

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

