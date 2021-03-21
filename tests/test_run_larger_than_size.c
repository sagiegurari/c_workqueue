#include "test.h"
#include "workqueue.h"
#include <stdlib.h>
#include <unistd.h>

size_t global_counter = 0;


void _run(void *args)
{
  assert_true(args == NULL);
  global_counter++;
}


void test_impl()
{
  struct WorkQueue *queue = workqueue_new_with_options(2);

  assert_true(queue != NULL);
  assert_size_equal(workqueue_get_queue_size(queue), 2);

  for (size_t index = 0; index < 50; index++)
  {
    assert_true(workqueue_push(queue, _run, NULL));
    workqueue_drain(queue);
  }

  assert_size_equal(workqueue_get_backlog_size(queue), 0);
  assert_size_equal(global_counter, 50);

  workqueue_release(queue);
}


int main()
{
  test_run(test_impl);
}

