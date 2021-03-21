#ifndef __TEST_H__
#define __TEST_H__

#include <stdbool.h>
#include <stddef.h>

void test_run(void (*fn)());
void test_fail();
void assert_true(bool);

void assert_size_equal(size_t, size_t);
void assert_num_equal(int, int);
void assert_string_equal(char *, char *);

#endif

