#include "test.h"
#include "workqueue.h"
#include <stdlib.h>
#include <unistd.h>

#define TEST_STRESS_LOOPS_COUNT    20
#define TEST_QUEUE_SIZE            60

int global_counter = 0;

struct FnTestArgs
{
  int counter;
};


void fn_null_args(void *args)
{
  assert_true(args == NULL);
}


void fn_sleep(void *args)
{
  assert_true(args == NULL);
  sleep(1);
}


void fn_args(void *args)
{
  assert_true(args != NULL);
  struct FnTestArgs *test_args = (struct FnTestArgs *)args;

  assert_num_equal(test_args->counter, global_counter);
  global_counter++;

  free(test_args);
}


void test_impl()
{
  struct WorkQueue *queue = workqueue_new_with_options(TEST_QUEUE_SIZE);

  assert_true(queue != NULL);
  assert_size_equal(workqueue_get_queue_size(queue), TEST_QUEUE_SIZE);

  for (size_t index = 0; index < TEST_STRESS_LOOPS_COUNT; index++)
  {
    assert_true(workqueue_push(queue, fn_null_args, NULL));
    assert_true(workqueue_push(queue, fn_sleep, NULL));

    struct FnTestArgs *test_args = malloc(sizeof(struct FnTestArgs));
    test_args->counter = index;
    assert_true(workqueue_push(queue, fn_args, test_args));
  }

  workqueue_drain(queue);

  assert_size_equal(workqueue_get_queue_size(queue), TEST_QUEUE_SIZE);
  assert_size_equal(workqueue_get_backlog_size(queue), 0);
  assert_num_equal(global_counter, TEST_STRESS_LOOPS_COUNT);

  workqueue_release(queue);
}


int main()
{
  test_run(test_impl);
}

