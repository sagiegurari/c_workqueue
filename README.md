# workqueue

[![CI](https://github.com/sagiegurari/c_workqueue/workflows/CI/badge.svg?branch=master)](https://github.com/sagiegurari/c_workqueue/actions)
[![Release](https://img.shields.io/github/v/release/sagiegurari/c_workqueue)](https://github.com/sagiegurari/c_workqueue/releases)
[![license](https://img.shields.io/github/license/sagiegurari/c_workqueue)](https://github.com/sagiegurari/c_workqueue/blob/master/LICENSE)

> Threaded work queue for C.

* [Overview](#overview)
* [Usage](#usage)
* [Contributing](.github/CONTRIBUTING.md)
* [Release History](CHANGELOG.md)
* [License](#license)

<a name="overview"></a>
## Overview
This library provides a work queue which enables to invoke provided functions in a background thread.<br>
Each queue owns a single background thread, so it is possible to create multiple queues for different purposes.

<a name="usage"></a>
## Usage

<!-- example source start -->
```c
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
```
<!-- example source end -->

## Contributing
See [contributing guide](.github/CONTRIBUTING.md)

<a name="history"></a>
## Release History

See [Changelog](CHANGELOG.md)

<a name="license"></a>
## License
Developed by Sagie Gur-Ari and licensed under the Apache 2 open source license.
