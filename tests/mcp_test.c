/* mcg_test.c - tests of the mcp pack functions
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#include <stdio.h>
#include <time.h>
#include <string.h>

/* NOTE: It is important that assert always aborts on failed assertion */
#undef NDEBUG
#include <assert.h>

#include <mcp_base/fbuf.h>
#include <mcp_base/mcp.h>

static void simple_test(void)
{
	struct mcp_parse buf;
	const unsigned char expected[] = {
		0x00,
		0xff,
		0x00, 0x00,
		0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00,
		0x01,
		0x01,

		0x80,
		0xff,
		0x00,
		0x01,
		0x7f,

		0x80, 0x00,
		0xff, 0xff,
		0x00, 0x00,
		0x00, 0x01,
		0x7f, 0xff,

		0x80, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01,
		0x7f, 0xff, 0xff, 0xff,

		0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
		0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

		0x00,
		0x01,
		0x80, 0x01,
		0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x01,
		0xff, 0xff, 0xff, 0xff, 0x0f,

		0x00,
		0x01,
		0x80, 0x01,
		0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,

		0xfe, 0xff, 0xff, 0xff, 0x0f,
		0x02,
		0x00,
		0x01,
		0xff, 0xff, 0xff, 0xff, 0x0f,

		0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
		0x02,
		0x00,
		0x01,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,

		't', 'e', 's', 't',
		0x04, 't', 'e', 's', 't',
		0x04, 't', 'e', 's', 't',

		0x41, 0x78, 0x00, 0x00,
		0x40, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	mcp_start(&buf, expected, sizeof(expected));

	assert(mcp_ubyte(&buf) == 0);
	assert(mcp_ubyte(&buf) == 0xff);
	assert(mcp_ushort(&buf) == 0);
	assert(mcp_ushort(&buf) == 0xffff);
	assert(mcp_uint(&buf) == 0);
	assert(mcp_uint(&buf) == 0xffffffffUL);
	assert(mcp_ulong(&buf) == 0);
	assert(mcp_ulong(&buf) == 0xffffffffffffffffULL);
	assert(mcp_bool(&buf) == 0);
	assert(mcp_bool(&buf) == 1);
	assert(mcp_bool(&buf) == 1);

	assert(mcp_byte(&buf) == -128);
	assert(mcp_byte(&buf) == -1);
	assert(mcp_byte(&buf) == 0);
	assert(mcp_byte(&buf) == 1);
	assert(mcp_byte(&buf) == 127);

	assert(mcp_short(&buf) == -0x7fff - 1);
	assert(mcp_short(&buf) == -1);
	assert(mcp_short(&buf) == 0);
	assert(mcp_short(&buf) == 1);
	assert(mcp_short(&buf) == 0x7fff);

	assert(mcp_int(&buf) == -0x7fffffff - 1);
	assert(mcp_int(&buf) == -1);
	assert(mcp_int(&buf) == 0);
	assert(mcp_int(&buf) == 1);
	assert(mcp_int(&buf) == 0x7fffffff);

	assert(mcp_long(&buf) == -0x7fffffffffffffffLL - 1);
	assert(mcp_long(&buf) == -1);
	assert(mcp_long(&buf) == 0);
	assert(mcp_long(&buf) == 1);
	assert(mcp_long(&buf) == 0x7fffffffffffffffLL);

	assert(mcp_varint(&buf) == 0);
	assert(mcp_varint(&buf) == 1);
	assert(mcp_varint(&buf) == (1 << 7));
	assert(mcp_varint(&buf) == (1 << 14));
	assert(mcp_varint(&buf) == (1 << 21));
	assert(mcp_varint(&buf) == (1 << 28));
	assert(mcp_varint(&buf) == 0xffffffffU);

	assert(mcp_varlong(&buf) == 0);
	assert(mcp_varlong(&buf) == 1);
	assert(mcp_varlong(&buf) == (1 << 7));
	assert(mcp_varlong(&buf) == (1 << 14));
	assert(mcp_varlong(&buf) == (1 << 21));
	assert(mcp_varlong(&buf) == (1 << 28));
	assert(mcp_varlong(&buf) == (1ULL << 35));
	assert(mcp_varlong(&buf) == (1ULL << 42));
	assert(mcp_varlong(&buf) == (1ULL << 49));
	assert(mcp_varlong(&buf) == (1ULL << 56));
	assert(mcp_varlong(&buf) == (1ULL << 63));
	assert(mcp_varlong(&buf) == 0xffffffffffffffffULL);

	assert(mcp_svarint(&buf) == 0x7fffffff);
	assert(mcp_svarint(&buf) == 1);
	assert(mcp_svarint(&buf) == 0);
	assert(mcp_svarint(&buf) == -1);
	assert(mcp_svarint(&buf) == (-0x7fffffff -1));

	assert(mcp_svarlong(&buf) == 0x7fffffffffffffffLL);
	assert(mcp_svarlong(&buf) == 1);
	assert(mcp_svarlong(&buf) == 0);
	assert(mcp_svarlong(&buf) == -1);
	assert(mcp_svarlong(&buf) == (-0x7fffffffffffffffLL-1));

	size_t szb, szc;
	const void *testa = mcp_raw(&buf, 4),
			*testb = mcp_bytes(&buf, &szb),
			*testc = mcp_bytes(&buf, &szc);

	assert(szb == 4 && szc == 4);
	assert(memcmp(testa, "test", 4) == 0);
	assert(memcmp(testb, "test", 4) == 0);
	assert(memcmp(testc, "test", 4) == 0);

	assert(mcp_float(&buf) == 0x1.fp3);
	assert(mcp_double(&buf) == 0x1.fp3);

	/* check that the operation succeeded without error*/
	assert(mcp_ok(&buf));
	assert(mcp_eof(&buf));
}

static void copy_test(void)
{
	struct mcp_parse buf;
	unsigned char temp[256];
	const unsigned char expected[] = {
		'a', 'b', 'c', 'd',
		0x04, 'e', 'f', 'g', 'h',
		0x04, 'i', 'j', 'k', 'l',
	};
	unsigned char expectedb[257] = {
		0xff, 0x01
	};
	int i;

	for (i = 0; i < 255; i++)
		expectedb[i + 2] = i;

	mcp_start(&buf, expected, sizeof(expected));

	memset(temp, 0xFE, sizeof(temp));
	assert(mcp_copy_raw(temp, &buf, 4) == 4);
	assert(memcmp(temp, "abcd", 4) == 0);

	memset(temp, 0xFE, sizeof(temp));
	assert(mcp_copy_bytes(temp, &buf, 0) == 4);
	assert(mcp_copy_bytes(temp, &buf, 4) == 4);
	assert(memcmp(temp, "efgh", 4) == 0);

	memset(temp, 0xFE, sizeof(temp));
	assert(mcp_copy_string((char*)temp, &buf, 0) == 5);
	assert(mcp_copy_string((char*)temp, &buf, 5) == 5);
	assert(memcmp(temp, "ijkl", 5) == 0);

	assert(mcp_ok(&buf));
	assert(mcp_eof(&buf));

	mcp_start(&buf, expectedb, sizeof(expectedb));
	memset(temp, 0xFE, sizeof(temp));
	assert(mcp_copy_bytes(temp, &buf, 0) == 255);
	assert(mcp_copy_bytes(temp, &buf, 255) == 255);
	for (i = 0; i < 255; i++)
		assert(temp[i] == i);

	assert(mcp_ok(&buf));
	assert(mcp_eof(&buf));

	mcp_start(&buf, expectedb, sizeof(expectedb));
	memset(temp, 0xFE, sizeof(temp));
	assert(mcp_copy_string((char*)temp, &buf, 0) == 256);
	assert(mcp_copy_string((char*)temp, &buf, 256) == 256);
	for (i = 0; i < 255; i++)
		assert(temp[i] == i);
	assert(temp[255] == 0);

	assert(mcp_ok(&buf));
	assert(mcp_eof(&buf));
}

#define NUM_TESTS		(4)
static void (*tests[NUM_TESTS])(void) = {simple_test, copy_test};
static const char *test_names[NUM_TESTS] = {"simple_test", "copy_test"};

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
