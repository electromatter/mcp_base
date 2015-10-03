#include "fbuf.h"
#include "mcp.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

/* NOTE: It is important that assert always aborts on failed assertion */
#undef NDEBUG
#include <assert.h>

static void hexdump(const void *data, size_t size)
{
	const char hex[16] = "0123456789abcdef";
	size_t offset;
	const unsigned char *ptr = data;
	
	/* works fine if the offset is less than 6 hex digits */
	
	for (offset = 0; offset < size; offset++) {
		if (offset > 0 && (offset & 0xf) == 0)
			fputc('\n', stderr);
		
		if ((offset & 0xf) == 0) {
			fputc(hex[(offset >> 20) & 0xf], stderr);
			fputc(hex[(offset >> 16) & 0xf], stderr);
			fputc(hex[(offset >> 12) & 0xf], stderr);
			fputc(hex[(offset >> 8) & 0xf], stderr);
			fputc(hex[(offset >> 4) & 0xf], stderr);
			fputc(hex[offset & 0xf], stderr);
			
			fputc(' ', stderr);
			fputc(' ', stderr);
		}
		
		fputc(hex[(ptr[offset] >> 4) & 0xf], stderr);
		fputc(hex[ptr[offset] & 0xf], stderr);
		fputc(' ', stderr);
		
		if ((offset & 0xf) == 7)
			fputc(' ', stderr);
	}
	if (size > 0)
		fputc('\n', stderr);
	
	fputc(hex[(offset >> 20) & 0xf], stderr);
	fputc(hex[(offset >> 16) & 0xf], stderr);
	fputc(hex[(offset >> 12) & 0xf], stderr);
	fputc(hex[(offset >> 8) & 0xf], stderr);
	fputc(hex[(offset >> 4) & 0xf], stderr);
	fputc(hex[offset & 0xf], stderr);
	fputc('\n', stderr);
	
	fflush(stderr);
}

static void simple_test(void)
{
	struct fbuf buf = FBUF_INITIALIZER;
	int err = 0;
	const unsigned char expected[] = {0x01, 0x02, 0x03, 0x04, 0x05,
										0x06, 0x07, 0x08, 0x09, 0x10,
										0x11, 0x12, 0x13, 0x14, 0x15,
										0x01, 0x00, 0xff, 0xff, 0xfe,
										0xff, 0xff, 0xff, 0xfd, 0xff,
										0xff, 0xff, 0xff, 0xff, 0xff,
										0xff, 0xfc};
	
	/* write out some tests */
	err |= mcg_ubyte(&buf, 0x01);
	err |= mcg_ushort(&buf, 0x0203);
	err |= mcg_uint(&buf, 0x04050607);
	err |= mcg_ulong(&buf, 0x0809101112131415);
	err |= mcg_bool(&buf, 12);
	err |= mcg_bool(&buf, 0);
	err |= mcg_byte(&buf, -1);
	err |= mcg_short(&buf, -2);
	err |= mcg_int(&buf, -3);
	err |= mcg_long(&buf, -4);
	
	/* TODO: edge cases */
	/* TODO: raw, bytes, raw_copy, bytes_copy */
	/* TODO: full buffer */
	
	/* print out the data for debugging */
	hexdump(fbuf_ptr(&buf), fbuf_avail(&buf));
	
	/* check that the operatoin succeeded without error*/
	assert(err == 0);
	
	/* test if the output is the same as what we expected */
	assert(fbuf_avail(&buf) == sizeof(expected));
	assert(memcmp(fbuf_ptr(&buf), expected, sizeof(expected)) == 0);
}

static void range_test(void)
{
	/* TODO: range test */
}

static void limit_test(void)
{
	/* TODO: float test */
}

static void float_test(void)
{
	/* TODO: float test */
}

#define NUM_TESTS		(4)
static void (*tests[NUM_TESTS])(void) = {simple_test, range_test, limit_test, float_test};
static const char *test_names[NUM_TESTS] = {"simple_test", "range_test", "limit_test", "float_test"};

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
