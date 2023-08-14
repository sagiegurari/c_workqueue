#include "threadlock.h"
#include "workqueue.h"
#include <pthread.h>
#include <stdlib.h>

struct WorkQueue
{
  bool                       released;
  bool                       started;
  bool                       draining;
  bool                       busy;
  size_t                     size;
  size_t                     current_index;
  size_t                     backlog_size;
  void(**fn_queue)(void *);
  void                       **args_queue;
  struct  WorkQueueThreadAPI *thread_api;
  bool                       release_thread_api;
  struct ThreadLock          *lock;
};

struct WorkQueueThreadAPIContext
{
  pthread_t work_thread;
};

static struct WorkQueueThreadAPI *_workqueue_create_thread_api(void);
static bool _workqueue_thread_api_start(struct WorkQueueThreadAPI *, void *(*fn)(void *), void *args);
static void _workqueue_thread_api_stop(struct WorkQueueThreadAPI *);
static void *_workqueue_loop(void *);

struct WorkQueue *workqueue_new(void)
{
  return(workqueue_new_with_options(WORKQUEUE_DEFAULT_QUEUE_SIZE, NULL));
}

struct WorkQueue *workqueue_new_with_options(size_t size, struct WorkQueueThreadAPI *thread_api)
{
  struct ThreadLockOptions lock_options = threadlock_new_default_options();

  lock_options.wait_timeout_in_milliseconds = 0;
  struct ThreadLock *lock = threadlock_new_with_options(lock_options);
  if (lock == NULL)
  {
    return(NULL);
  }

  struct WorkQueue *queue = malloc(sizeof(struct WorkQueue));

  queue->released           = false;
  queue->started            = false;
  queue->draining           = false;
  queue->busy               = false;
  queue->size               = size > 0 ? size : WORKQUEUE_DEFAULT_QUEUE_SIZE;
  queue->backlog_size       = 0;
  queue->current_index      = 0;
  queue->fn_queue           = malloc(sizeof(void (*)(void *)) * queue->size);
  queue->args_queue         = malloc(sizeof(void *) * queue->size);
  queue->thread_api         = thread_api;
  queue->release_thread_api = thread_api == NULL;
  queue->lock               = lock;

  if (queue->thread_api == NULL)
  {
    queue->thread_api = _workqueue_create_thread_api();
  }

  return(queue);
}


void workqueue_release(struct WorkQueue *queue)
{
  if (queue == NULL)
  {
    return;
  }

  if (queue->started)
  {
    // mark as released so thread will stop processing
    // the queue after finishing the current item
    threadlock_lock(queue->lock);
    queue->released = true;
    threadlock_signal(queue->lock, false /* lock */);
    threadlock_unlock(queue->lock);

    queue->thread_api->stop(queue->thread_api);
  }

  threadlock_release(queue->lock);
  free(queue->fn_queue);
  free(queue->args_queue);
  if (queue->release_thread_api)
  {
    free(queue->thread_api->context);
    free(queue->thread_api);
  }
  free(queue);
}


size_t workqueue_get_queue_size(struct WorkQueue *queue)
{
  if (queue == NULL)
  {
    return(0);
  }

  return(queue->size);
}


size_t workqueue_get_backlog_size(struct WorkQueue *queue)
{
  if (queue == NULL)
  {
    return(0);
  }

  threadlock_lock(queue->lock);
  size_t size = queue->backlog_size;
  threadlock_unlock(queue->lock);

  return(size);
}


bool workqueue_is_busy(struct WorkQueue *queue)
{
  if (queue == NULL)
  {
    return(false);
  }

  // no lock to get current state without waiting
  return(queue->busy);
}


bool workqueue_push(struct WorkQueue *queue, void (*fn)(void *), void *args)
{
  if (queue == NULL || fn == NULL || queue->draining)
  {
    return(false);
  }

  threadlock_lock(queue->lock);

  if (!queue->started)
  {
    // start the background work thread
    if (!queue->thread_api->start(queue->thread_api, _workqueue_loop, queue))
    {
      threadlock_unlock(queue->lock);
      return(false);
    }

    queue->started = true;
  }

  if (queue->backlog_size == queue->size)
  {
    threadlock_unlock(queue->lock);
    return(false);
  }

  size_t next_index = (queue->current_index + queue->backlog_size) % queue->size;
  queue->fn_queue[next_index]   = fn;
  queue->args_queue[next_index] = args;
  queue->backlog_size++;

  threadlock_signal(queue->lock, false /* lock */);

  threadlock_unlock(queue->lock);

  return(true);
}


void workqueue_drain(struct WorkQueue *queue)
{
  if (queue == NULL || !queue->started || !queue->backlog_size)
  {
    return;
  }

  threadlock_lock(queue->lock);
  queue->draining = true;
  threadlock_signal(queue->lock, false /* lock */);
  threadlock_wait(queue->lock, false /* lock */);
  queue->draining = false;
  threadlock_unlock(queue->lock);
}

static struct WorkQueueThreadAPI *_workqueue_create_thread_api(void)
{
  struct WorkQueueThreadAPI *thread_api = malloc(sizeof(struct WorkQueueThreadAPI));

  thread_api->start   = _workqueue_thread_api_start;
  thread_api->stop    = _workqueue_thread_api_stop;
  thread_api->context = malloc(sizeof(struct WorkQueueThreadAPIContext));

  return(thread_api);
}


static bool _workqueue_thread_api_start(struct WorkQueueThreadAPI *thread_api, void *(*fn)(void *), void *args)
{
  if (thread_api == NULL)
  {
    return(false);
  }

  struct WorkQueueThreadAPIContext *context = (struct WorkQueueThreadAPIContext *)thread_api->context;

  return(!pthread_create(&context->work_thread, NULL, fn, args));
}


static void _workqueue_thread_api_stop(struct WorkQueueThreadAPI *thread_api)
{
  if (thread_api == NULL)
  {
    return;
  }

  struct WorkQueueThreadAPIContext *context = (struct WorkQueueThreadAPIContext *)thread_api->context;

  pthread_join(context->work_thread, NULL);
}


static void *_workqueue_loop(void *thread_args)
{
  if (thread_args == NULL)
  {
    return(NULL);
  }

  struct WorkQueue *queue = (struct WorkQueue *)thread_args;

  while (!queue->released)
  {
    threadlock_lock(queue->lock);

    if (queue->backlog_size)
    {
      void (*fn)(void *) = queue->fn_queue[queue->current_index];
      void *args = queue->args_queue[queue->current_index];

      queue->current_index = (queue->current_index + 1) % queue->size;
      queue->backlog_size--;

      queue->busy = true;
      fn(args);
      // cppcheck-suppress redundantAssignment
      queue->busy = false;
    }
    else
    {
      threadlock_wait(queue->lock, false /* lock */);
    }

    if (!queue->backlog_size)
    {
      threadlock_signal(queue->lock, false /* lock */);
    }

    threadlock_unlock(queue->lock);
  }

  return(NULL);
}

