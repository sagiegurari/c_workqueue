#include "test.h"
#include "workqueue.h"
#include <pthread.h>
#include <stdlib.h>

static int    _test_started  = 0;
static int    _test_stopped  = 0;
static size_t global_counter = 0;


void _run(void *args)
{
  assert_true(args == NULL);
  global_counter++;
}


static bool _test_thread_api_start(struct WorkQueueThreadAPI *thread_api, void *(*fn)(void *), void *args)
{
  if (thread_api == NULL)
  {
    return(false);
  }

  pthread_t thread = (pthread_t)thread_api->context;
  _test_started = _test_started + 1;

  return(pthread_create(&thread, NULL, fn, args) == 0);
}


static void _test_thread_api_stop(struct WorkQueueThreadAPI *thread_api)
{
  if (thread_api == NULL)
  {
    return;
  }

  pthread_t thread = (pthread_t)thread_api->context;

  pthread_join(thread, NULL);

  _test_stopped = _test_stopped + 1;
}


void test_impl()
{
  struct WorkQueueThreadAPI *thread_api = malloc(sizeof(struct WorkQueueThreadAPI));
  pthread_t                 work_thread;

  thread_api->start   = _test_thread_api_start;
  thread_api->stop    = _test_thread_api_stop;
  thread_api->context = &work_thread;

  struct WorkQueue *queue = workqueue_new_with_options(15, thread_api);

  assert_true(queue != NULL);
  assert_size_equal(workqueue_get_queue_size(queue), 15);
  assert_size_equal(_test_started, 0);
  assert_size_equal(_test_stopped, 0);

  for (size_t index = 0; index < 15; index++)
  {
    assert_true(workqueue_push(queue, _run, NULL));
  }
  workqueue_drain(queue);

  assert_size_equal(workqueue_get_backlog_size(queue), 0);
  assert_size_equal(global_counter, 15);

  assert_size_equal(_test_started, 1);
  workqueue_release(queue);
  assert_size_equal(_test_stopped, 1);
}


int main()
{
  test_run(test_impl);
}

