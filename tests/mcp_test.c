#include <stdlib.h>
#include "fbuf.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>

/* keep iterations large enough for a decent chance of catching bugs,
 * but small enough to fit on a machine without causing overflows 
 *
 * the maximum ammount of memory that random_test can take is
 * 	RANDOM_ITERATIONS * RANDOM_MAX_SIZE
 * 
 * Keep the above product under the limit of the int type on your system.
 */
#define RANDOM_ITERATIONS		(10000)
#define RANDOM_MAX_SIZE			(1000)

static int simple_test(void)
{
	struct fbuf buf;
	fbuf_init(&buf, FBUF_MAX);
	
	fbuf_wptr(&buf, 10000);
	fbuf_produce(&buf, 10000);
	
	fbuf_free(&buf);
	
	return 0;
}

static int random_test(void)
{
	struct fbuf buf = FBUF_INITIALIZER;
	const unsigned char *base;
	unsigned char *wbase;
	int i, j, size, valid = 0, start = 0, end = 0;
	
	/* seed the rng */
	srand(time(NULL));
	
	for (i = 0; i < RANDOM_ITERATIONS; i++) {
		/* pick a random block size */
		size = rand() % RANDOM_MAX_SIZE;
		wbase = fbuf_wptr(&buf, size);
		if (wbase == NULL) {
			assert(0);
			return 1;
		}
		
		/* write the pattern: 1, 2, 3, ... 255, 1, 2 ... */
		for (j = 0; j < size; j++)
			wbase[j] = (j + end) & 0xff;
		end += size;
		fbuf_produce(&buf, size);
		valid += size;
		
		/* consume a random block size */
		size = rand() % RANDOM_MAX_SIZE;
		if (size > valid)
			size = valid;
		fbuf_consume(&buf, size);
		valid -= size;
		start += size;
	}
	
	/* verify that the buffer contains the correct data */
	assert(valid == fbuf_avail(&buf));
	base = fbuf_ptr(&buf);
	for (j = 0; j < size; j++) {
		if (base[j] != ((j + start) & 0xff)) {
			assert(0);
			return 1;
		}
	}
	
	/* try to compact it, and reverify */
	fbuf_compact(&buf);
	assert(valid == fbuf_avail(&buf));
	assert(buf.start == 0);
	base = fbuf_ptr(&buf);
	for (j = 0; j < size; j++) {
		if (base[j] != ((j + start) & 0xff)) {
			assert(0);
			return 1;
		}
	}
	
	fbuf_free(&buf);
	
	return 0;
}

static int limit_test(void)
{
	struct fbuf buf;
	size_t request, ret;
	fbuf_init(&buf, 16800);
	
	/* try expanding the buffer a few times */
	request = 1000;
	ret = fbuf_expand(&buf, request);
	if (ret != fbuf_wavail(&buf) || ret < request) {
		assert(0);
		return 1;
	}
	
	request = 10000;
	ret = fbuf_expand(&buf, request);
	if (ret != fbuf_wavail(&buf) || ret < request) {
		assert(0);
		return 1;
	}
	
	request = 16384;
	ret = fbuf_expand(&buf, request);
	if (ret != fbuf_wavail(&buf) || ret < request) {
		assert(0);
		return 1;
	}
	
	/* now try one that should fail */
	request = 100000;
	ret = fbuf_expand(&buf, request);
	if (ret != fbuf_wavail(&buf) || ret >= request) {
		assert(0);
		return 1;
	}
	
	/* lift the limit */
	buf.max_size = FBUF_MAX;
	
	/* see what happens when we try something that should overflow */
	request = FBUF_MAX;
	ret = fbuf_expand(&buf, request);
	if (ret != fbuf_wavail(&buf) || ret >= request) {
		assert(0);
		return 1;
	}
	
	fbuf_free(&buf);
	
	return 0;
}

#define NUM_TESTS		(3)
static int (*tests[NUM_TESTS])(void) = {simple_test,
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

