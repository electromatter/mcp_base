#include <stdlib.h>
#include "fbuf.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>

static int limit_test(void)
{
	return 0;
}

#define NUM_TESTS		(0)
static int (*tests[NUM_TESTS])(void) = {};
static const char *test_names[NUM_TESTS] = {};

static int print_usage();

static int do_test(int test_num)
{
	if (test_num < 0 || test_num >= NUM_TESTS)
		return print_usage();
	fprintf(stderr, "starting subtest: %s\n", test_names[test_num]);
	fflush(stderr);
	return tests[test_num]();
}

static int print_usage()
{
	int i;
	fprintf(stderr, "usage: ./test <subtest_no> <subtest_no> ...\n");
	fprintf(stderr, "subtests:\n");
	for (i = 0; i < NUM_TESTS; i++)
		fprintf(stderr, "\t%i\t%s\n", i, test_names[i]);
	fflush(stderr);
	return 1;
}

int main(int argc, char **argv)
{
	int test, i, ret;

	for (i = 1; i < argc; i++) {
		if (sscanf(argv[i], "%i", &test) != 1)
			return print_usage();
		ret = do_test(test);
		if (ret)
			return ret;
	}

	if (argc < 2)
		return print_usage();

	return 0;
}

