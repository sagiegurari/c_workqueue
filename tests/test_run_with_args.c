#include "test.h"
#include "workqueue.h"
#include <stdlib.h>
#include <unistd.h>


void _run(void *args)
{
  assert_string_equal("A", (char *)args);
}


void test_impl()
{
  struct WorkQueue *queue = workqueue_new();

  assert_true(queue != NULL);

  char *args = malloc(sizeof(char) * 2);
  args[0] = 'A';
  args[1] = 0;
  assert_true(workqueue_push(queue, _run, args));
  workqueue_drain(queue);

  free(args);

  assert_size_equal(workqueue_get_backlog_size(queue), 0);

  workqueue_release(queue);
}


int main()
{
  test_run(test_impl);
}

