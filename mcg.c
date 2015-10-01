#include <stdlib.h>

#include "mcp_types.h"

#include "fbuf.h"
#include "mcp.h"

#include <assert.h>

/* returns the number of bits in an integer type */
#define NUM_BITS(type)          (8 * sizeof(type))
/* returns an integer with a number of ones set starting from the lsb */
#define ALL_ONES(num_bits)      ((~(mcp_ulong_t)0) >>				\
									(NUM_BITS(mcp_ulong_t) - (num_bits)))

/* Enabling this causes mcg to use type-punning when interpreting floats */
#ifndef USE_UNSAFE_MCP_FLOAT
# define USE_UNSAFE_MCP_FLOAT			0
#endif

/* Enable this on a two's complement machine. It is faster. */
#ifndef USE_MCP_TWOS_COMP
# define USE_MCP_TWOS_COMP				0
#endif

#if !USE_UNSAFE_MCP_FLOAT
# include <math.h>
#endif

/* This is to aid in debugging. Enabling this raises an assertion failure
 * when an invalid value is passed to mcg */
#ifndef USE_MCP_ASSERT_OVERFLOW
# define USE_MCP_ASSERT_OVERFLOW	0
#endif

/*TODO: change large integer constants; add UL/ULL */

/* for compatibility with varint28 */
#define MCG_BYTES_MAX_SIZE		(ALL_ONES(28))

int mcg_raw(struct fbuf *buf, const void *data, size_t size)
{
	/* copy the raw data into the buffer */
	if (fbuf_copy(buf, data, size) != size)
		return 1;
	return 0;
}

int mcg_varint(struct fbuf *buf, mcp_varint_t value)
{
	const int max_result_size = (NUM_BITS(value) + 6) / 7;
	unsigned char *dest = fbuf_wptr(buf, max_result_size);
	int i = 0;

	/* we could not allocate enough space. */
	if (dest == NULL)
		return 1;

	do {
		/* write out 7 bits */
		dest[i] = value & 0x7f;

		/* if the value is larger than 7 bits,
		 * then mark that there are more bytes to follow */
		if (value > 0x7f)
			dest[i] |= 0x80;

		/* advance to the next byte */
		i++;
		value >>= 7;

		/* if there are no more bits then stop*/
	} while(value > 0);
	return 0;
}

int mcg_varlong(struct fbuf *buf, mcp_varlong_t value)
{
	const int max_result_size = (NUM_BITS(value) + 6) / 7;
	unsigned char *dest = fbuf_wptr(buf, max_result_size);
	int i = 0;

	/* we could not allocate enough space. */
	if (dest == NULL)
		return 1;

	do {
		/* write out 7 bits */
		dest[i] = value & 0x7f;

		/* if the value is larger than 7 bits,
		 * then mark that there are more bytes to follow */
		if (value > 0x7f)
			dest[i] |= 0x80;

		/* advance to the next byte */
		i++;
		value >>= 7;

		/* if there are no more bits then stop*/
	} while(value > 0);
	return 0;
}

int mcg_svarint(struct fbuf *buf, mcp_svarint_t value)
{
	mcp_varint_t packed = (mcp_varint_t)value << 2;

	/* pack sign value */
	if (value < 0)
		packed = (((mcp_varint_t)-(value + 1)) << 2) | 1;

	return mcg_varlong(buf, packed);
}

int mcg_svarlong(struct fbuf *buf, mcp_svarlong_t value)
{
	mcp_varlong_t packed = (mcp_varlong_t)value << 2;

	/* pack sign value */
	if (value < 0)
		value = (((mcp_varlong_t)-(value + 1)) << 2) | 1;

	/* write it out */
	return mcg_varlong(buf, packed);
}

int mcg_bytes(struct fbuf *buf, const void *value, size_t size)
{
	/* overflow check */
	if (size > MCG_BYTES_MAX_SIZE)
		return 1;

	/* write size prefix */
	if (mcg_varint(buf, size))
		return 1;

	/* copy data */
	return mcg_raw(buf, value, size);
}

int mcg_ubyte(struct fbuf *buf, mcp_ubyte_t value)
{
	unsigned char data[1];

#if USE_MCP_ASSERT_OVERFLOW
	assert_int(value <= ALL_ONES(8));
#endif

	/* overflow check */
	if (value > ALL_ONES(8))
		return 1;

	/*write value*/
	data[0] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_ushort(struct fbuf *buf, mcp_ushort_t value)
{
	unsigned char data[2];

#if USE_MCP_ASSERT_OVERFLOW
	assert_int(value <= ALL_ONES(16));
#endif

	/* overflow check */
	if (value > ALL_ONES(16))
		return 1;

	/* write value*/
	data[0] = (value >> 8) & 0xff;
	data[1] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_uint(struct fbuf *buf, mcp_uint_t value)
{
	unsigned char data[4];

#if USE_MCP_ASSERT_OVERFLOW
	assert_int(value <= ALL_ONES(32));
#endif

	/* overflow check */
	if (value > ALL_ONES(32))
		return 1;

	/* write value */
	data[0] = (value >> 24) & 0xff;
	data[1] = (value >> 16) & 0xff;
	data[2] = (value >> 8) & 0xff;
	data[3] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_ulong(struct fbuf *buf, mcp_ulong_t value)
{
	unsigned char data[8];

#if USE_MCP_ASSERT_OVERFLOW
	assert_int(value <= ALL_ONES(64));
#endif

	/* overflow check */
	if (value > ALL_ONES(64))
		return 1;

	/* write value */
	data[0] = (value >> 56) & 0xff;
	data[1] = (value >> 48) & 0xff;
	data[2] = (value >> 40) & 0xff;
	data[3] = (value >> 32) & 0xff;
	data[4] = (value >> 24) & 0xff;
	data[5] = (value >> 16) & 0xff;
	data[6] = (value >> 8) & 0xff;
	data[7] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_byte(struct fbuf *buf, mcp_byte_t value)
{
#if USE_MCP_ASSERT_OVERFLOW
	assert(value < ALL_ONES(7) && value >= -ALL_ONES(7) - 1);
#endif

	/* overflow check */
	if (value > ALL_ONES(7) || value < -ALL_ONES(7) - 1)
		return 1;

#if USE_MCP_TWOS_COMP
	return mcg_ubyte(buf, value);
#else
	/* pack two's complement */
	if (value < 0)
		return mcg_ubyte(buf, 0x80 | (mcp_ubyte_t)-(value + 1));

	return mcg_ubyte(buf, value);
#endif
}

int mcg_short(struct fbuf *buf, mcp_short_t value)
{
#if USE_MCP_ASSERT_OVERFLOW
	assert(value < ALL_ONES(15) && value >= -ALL_ONES(15) - 1);
#endif

	/* overflow check */
	if (value > ALL_ONES(15) || value < -ALL_ONES(15) - 1)
		return 1;

#if USE_MCP_TWOS_COMP
	return mcg_ushort(buf, value);
#else
	if (value < 0)
		return mcg_ubyte(buf, 0x8000 | (mcp_ushort_t)-(value + 1));
	return mcg_ubyte(buf, value);
#endif
}

int mcg_int(struct fbuf *buf, mcp_int_t value)
{
#if USE_MCP_ASSERT_OVERFLOW
	assert(value < ALL_ONES(31) && value >= -ALL_ONES(31) - 1);
#endif

	/* overflow check */
	if (value > ALL_ONES(31) || value < -ALL_ONES(31) - 1)
		return 1;

#if USE_MCP_TWOS_COMP
	return mcg_uint(buf, value);
#else
	if (value < 0)
		return mcg_ubyte(buf, 0x80000000 | (mcp_uint_t)-(value + 1));
	return mcg_ubyte(buf, value);
#endif
}

int mcg_long(struct fbuf *buf, mcp_long_t value)
{
#if USE_MCP_ASSERT_OVERFLOW
	assert(value < ALL_ONES(63) && value >= -ALL_ONES(63) - 1);
#endif

	/* overflow check */
	if (value > ALL_ONES(63) || value < -ALL_ONES(63) - 1)
		return 1;

#if USE_MCP_TWOS_COMP
	return mcg_ulong(buf, value);
#else
	if (value < 0)
		return mcg_ubyte(buf, 0x8000000000000000 | (mcp_ulong_t)-(value + 1));
	return mcg_ubyte(buf, value);
#endif
}

int mcg_bool(struct fbuf *buf, mcp_bool_t value)
{
	return mcg_byte(buf, !!value);
}

int mcg_float(struct fbuf *buf, float x)
{
#if USE_UNSAFE_MCP_FLOAT
	union {
		mcp_uint_t i;
	    float f;
	} value;
	assert(sizeof(mcp_uint_t) == sizeof(float));
	value.f = x;
	return mcg_uint(buf, value.i);
#else
	mcp_uint_t result = 0;
	int exp;
	double significand;
	if (x < 0) {
		x = -x;
		result |= 0x80000000;
	}
	significand = frexp(x, &exp);
	exp -= 1;
	if (exp < 0)
		exp = -exp - 1;
	exp &= ALL_ONES(8);
	result |= exp << 23;
	result |= (mcp_uint_t)(significand * (1 << 24)) & ALL_ONES(23);
	return mcg_uint(buf, result);
#endif
}

int mcg_double(struct fbuf *buf, double x)
{
#if USE_UNSAFE_MCP_FLOAT
	union {
		mcp_ulong_t i;
		double f;
	} value;
	assert(sizeof(mcp_uint_t) == sizeof(float));
	value.f = x;
	return mcg_uint(buf, value.i);
#else
	mcp_ulong_t result = 0;
	int exp;
	double significand;
	if (x < 0) {
		x = -x;
		result |= 0x8000000000000000;
	}
	significand = frexp(x, &exp);
	exp -= 1;
	if (exp < 0)
		exp = -exp - 1;
	exp &= ALL_ONES(11);
	result |= (mcp_ulong_t)exp << 52;
	result |= (mcp_uint_t)(significand * (1L << 53)) & ALL_ONES(52);
	return mcg_ulong(buf, result);
#endif
}

