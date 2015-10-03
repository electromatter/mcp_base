/* fbuf_test.c - tests of the fbuf implementation
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#include "fbuf.h"

#include <stdio.h>
#include <time.h>

/* NOTE: It is important that assert always aborts on failed assertion */
#undef NDEBUG
#include <assert.h>

/* keep iterations large enough for a decent chance of catching bugs,
 * but small enough to fit on a machine without causing overflows 
 *
 * the maximum ammount of memory that random_test can take is
 * 	RANDOM_ITERATIONS * RANDOM_MAX_SIZE
 * 
 * Keep the above product under the limit of the int type on your system.
 */
#define RANDOM_ITERATIONS		(100000)
#define RANDOM_MAX_SIZE			(1000)

static void simple_test(void)
{
	struct fbuf buf;
	fbuf_init(&buf, FBUF_MAX);
	
	/* produce simple test */
	assert(fbuf_wptr(&buf, 1024));
	assert(fbuf_wavail(&buf) >= 1024);
	assert(fbuf_avail(&buf) == 0);
	fbuf_produce(&buf, 1024);
	assert(fbuf_avail(&buf) == 1024);
	
	/* clear test */
	fbuf_clear(&buf);
	assert(fbuf_avail(&buf) == 0);
	
	/* free test */
	fbuf_free(&buf);
	assert(fbuf_avail(&buf) == 0);
	assert(fbuf_wavail(&buf) == 0);
	assert(buf.size == 0);
	assert(buf.base == NULL);
	
	/* compact test */
	assert(fbuf_wptr(&buf, 1024));
	assert(fbuf_wavail(&buf) >= 1024);
	assert(fbuf_avail(&buf) == 0);
	fbuf_produce(&buf, 1024);
	assert(fbuf_avail(&buf) == 1024);
	
	fbuf_compact(&buf);
	assert(fbuf_avail(&buf) == 1024);
	
	fbuf_consume(&buf, 512);
	assert(fbuf_avail(&buf) == 512);
	
	fbuf_compact(&buf);
	assert(buf.start == 0);
	assert(fbuf_avail(&buf) == 512);
	
	fbuf_consume(&buf, 256);
	assert(fbuf_avail(&buf) == 256);
	
	fbuf_consume(&buf, 256);
	assert(buf.start == 0);
	assert(fbuf_avail(&buf) == 0);
	
	buf.max_size = buf.size;
	
	/* produce consume tests */
	fbuf_produce(&buf, 512);
	assert(buf.start == 0);
	assert(buf.end == 512);
	
	fbuf_consume(&buf, 256);
	assert(buf.start == 256);
	assert(buf.end == 512);
	
	fbuf_produce(&buf, fbuf_wavail(&buf));
	assert(fbuf_wavail(&buf) == 0);
	assert(buf.start == 256);
	assert(buf.end == buf.size);
	
	fbuf_consume(&buf, fbuf_avail(&buf));
	assert(fbuf_avail(&buf) == 0);
	assert(fbuf_wavail(&buf) == buf.size);
	assert(buf.start == 0);
	assert(buf.end == 0);
	
	fbuf_free(&buf);
}

static void random_test(void)
{
	struct fbuf buf = FBUF_INITIALIZER;
	const unsigned char *base;
	unsigned char *wbase;
	unsigned int i, j, size, valid = 0, start = 0, end = 0;
	
	/* seed the rng */
	srand(time(NULL));
	
	for (i = 0; i < RANDOM_ITERATIONS; i++) {
		/* pick a random block size */
		size = rand() % RANDOM_MAX_SIZE;
		wbase = fbuf_wptr(&buf, size);
		assert(wbase);
		
		/* write the pattern: 0, 1, 2, ... 255, 0, 1, 2 ... */
		for (j = 0; j < size; j++)
			wbase[j] = (j + end) & 0xff;
		valid += size;
		end = (end + size) & 0xff;
		fbuf_produce(&buf, size);
		
		/* consume a random block size */
		size = rand() % RANDOM_MAX_SIZE;
		if (size > valid)
			size = valid;
		fbuf_consume(&buf, size);
		valid -= size;
		start = (start + size) & 0xff;
	}
	
	/* verify that the buffer contains the correct data */
	assert(valid == fbuf_avail(&buf));
	base = fbuf_ptr(&buf);
	for (j = 0; j < valid; j++)
		assert(base[j] == ((j + start) & 0xff));
	
	/* try to compact it, and reverify */
	fbuf_compact(&buf);
	assert(valid == fbuf_avail(&buf));
	assert(buf.start == 0);
	base = fbuf_ptr(&buf);
	for (j = 0; j < valid; j++)
		assert(base[j] == ((j + start) & 0xff));
	
	fbuf_free(&buf);
}

static void limit_test(void)
{
	struct fbuf buf;
	size_t request, ret;
	fbuf_init(&buf, 16800);
	
	/* try expanding the buffer a few times */
	request = 1000;
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret >= request);
	
	request = 10000;
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret >= request);
	
	request = 16384;
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret >= request);
	
	/* now try one that should fail */
	request = 100000;
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret < request);
	
	/* this should succeed */
	request = 16700;
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret >= request);
	
	/* and up to the limit */
	request = 16800;
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret >= request);
	
	/* lift the limit */
	assert(!fbuf_shrink(&buf, FBUF_MAX));
	assert(buf.max_size == FBUF_MAX);
	
	/* see what happens when we try something that should overflow */
	request = FBUF_MAX >> 2; /* to avoid a valgrind warning of malloc(-1);*/
	ret = fbuf_expand(&buf, request);
	assert(ret == fbuf_wavail(&buf) && ret < request);
	
	/* put some garbage data in the fbuf */
	fbuf_produce(&buf, fbuf_wavail(&buf));
	
	/* shrink to less than what is waiting should fail */
	assert(fbuf_shrink(&buf, 0));
	assert(buf.max_size == FBUF_MAX);
	
	/* shrink down to larger should succeed */
	assert(!fbuf_shrink(&buf, 18000));
	assert(buf.max_size == 18000);
	
	/* shrink down to exactly what is waiting should succeed */
	assert(!fbuf_shrink(&buf, 16800));
	assert(buf.max_size == 16800);
	
	/* clear out that data so we can try some more shrinks */
	fbuf_clear(&buf);
	
	/* shrink down to exactly what is waiting should succeed */
	assert(!fbuf_shrink(&buf, 0));
	assert(buf.max_size == 0);
	
	/* shrink back to default should succeed */
	assert(!fbuf_shrink(&buf, FBUF_MAX));
	assert(buf.max_size == FBUF_MAX);
	
	fbuf_free(&buf);
}

#define NUM_TESTS		(3)
static void (*tests[NUM_TESTS])(void) = {simple_test,
										random_test,
										limit_test};
static const char *test_names[NUM_TESTS] = {"simple_test",
											"random_test",
											"limit_test"};

static int print_usage();

static int do_test(int test_num)
{
	if (test_num < 0 || test_num >= NUM_TESTS)
		return print_usage();
	fprintf(stderr, "starting subtest: %s\n", test_names[test_num]);
	fflush(stderr);
	tests[test_num]();
	return 0;
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
