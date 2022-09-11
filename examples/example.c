#include "workqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct FnArgs
{
  int counter;
};

void work_fn(void *);


int main()
{
  printf("Library Examples:\n");

  // create new threaded work queue with default queue size
  // workqueue_new_with_options can be used to define custom queue size and threading api.
  // each queue owns its own background thread and its possible to create many queues in parallel.
  struct WorkQueue *queue = workqueue_new();

  printf("Queue Size: %zu Backlog Size: %zu\n", workqueue_get_queue_size(queue), workqueue_get_backlog_size(queue));

  for (size_t index = 0; index < 20; index++)
  {
    struct FnArgs *args = malloc(sizeof(struct FnArgs));
    args->counter = index;

    if (!workqueue_push(queue, work_fn, args))
    {
      printf("Failed to push work function to queue\n");
      free(args);
    }
  }

  printf("Backlog Size: %zu\n", workqueue_get_backlog_size(queue));

  // wait for queue to finish, queue can still be used afterwards
  workqueue_drain(queue);
  printf("Backlog Size: %zu\n", workqueue_get_backlog_size(queue));

  // release when done
  workqueue_release(queue);

  return(0);
}


void work_fn(void *args)
{
  struct FnArgs *fn_args = (struct FnArgs *)args;

  sleep(1);
  printf("Counter: %d\n", fn_args->counter);

  free(fn_args);
}

