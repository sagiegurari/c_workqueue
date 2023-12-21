#include "test.h"
#include "workqueue.h"
#include <stdlib.h>
#include <unistd.h>

bool _global_started = false;


void _run(void *args)
{
  _global_started = true;
  assert_true(args == NULL);
  sleep(3);
}


void test_impl()
{
  struct WorkQueue *queue = workqueue_new();

  assert_true(queue != NULL);

  assert_true(!_global_started);
  assert_true(workqueue_push(queue, _run, NULL));
  sleep(1);
  assert_true(_global_started);
  workqueue_drain(queue);

  assert_size_equal(workqueue_get_backlog_size(queue), 0);

  workqueue_release(queue);
}


int main()
{
  test_run(test_impl);
}

