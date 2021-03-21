#include "test.h"
#include "workqueue.h"
#include <stdlib.h>
#include <unistd.h>


void _run(void *args)
{
  assert_true(args == NULL);
}


void test_impl()
{
  struct WorkQueue *queue = workqueue_new();

  assert_true(queue != NULL);

  assert_true(workqueue_push(queue, _run, NULL));
  workqueue_drain(queue);

  assert_size_equal(workqueue_get_backlog_size(queue), 0);

  workqueue_release(queue);
}


int main()
{
  test_run(test_impl);
}

