#include "test.h"
#include "workqueue.h"
#include <stdlib.h>


void test_impl()
{
  struct WorkQueue *queue = workqueue_new();

  assert_true(queue != NULL);
  assert_size_equal(workqueue_get_queue_size(queue), 250);

  workqueue_release(queue);
}


int main()
{
  test_run(test_impl);
}

