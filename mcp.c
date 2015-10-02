#include <stdlib.h>
#include "mcp_types.h"
#include "mcp.h"

#include <string.h>
#include <assert.h>

/* returns the number of bits in an integer type */
#define NUM_BITS(type)			(8 * sizeof(type))
/* returns an integer with a number of ones set starting from the lsb */
#define ALL_ONES(num_bits)		((~(mcp_ulong_t)0) >>					\
									(NUM_BITS(mcp_long_t) - (num_bits)))
/* returns a long with bit number bit_num set, counting from the lsb */
#define ONE_BIT(bit_num)		(((mcp_ulong_t)1) << (bit_num))

/* returns the number of bytes that we can access from mcp_ptr */
#define mcp_avail(buf)					((buf)->end - (buf)->start)
/* returns the base pointer of the buffer */
#define mcp_ptr(buf)					(&(buf)->base[(buf)->start])
/* advances the offset; consuming a number of bytes */
#define mcp_consume(buf, size)			do {(buf)->start += (size);} while (0);

#ifndef MCP_SYSTEM_IEEE754_FLOAT
# define MCP_SYSTEM_IEEE754_FLOAT 0
#endif

#if !MCP_SYSTEM_IEEE754_FLOAT
# include <math.h>
#endif

const void *mcp_raw(struct mcp_parse *buf, size_t size)
{
	const void *ret;

	/* pass errors */
	if (!mcp_ok(buf))
		return NULL;

	/* bounds check */
	if (mcp_avail(buf) < size) {
		buf->error = MCP_EOVERRUN;
		return NULL;
	}

	/* get the pointer and consume our raw data */
	ret = mcp_ptr(buf);
	mcp_consume(buf, size);

	return ret;
}

void mcp_copy_raw(void *dest, struct mcp_parse *buf, size_t size)
{
	const void *value = mcp_raw(buf, size);

	/* pass errors */
	if (!mcp_ok(buf))
		return;

	memcpy(dest, value, size);
}

mcp_varint_t mcp_varint(struct mcp_parse *buf)
{
	mcp_varint_t byte, ret = 0;
	const unsigned int num_bits = 32,
			max_shift = num_bits - 7,
			final_max = ALL_ONES(max_shift % 7);
	unsigned int shift = 0;

	/* pass errors */
	if (!mcp_ok(buf))
		return ret;

	while (1) {
		/* check if we are in bounds */
		if (mcp_avail(buf) <= 0) {
			buf->error = MCP_EOVERRUN;
			return ret;
		}

		byte = mcp_ptr(buf)[0];

		/* check if this would cause an overflow */
		if (shift > max_shift && byte > final_max) {
			buf->error = MCP_EOVERFLOW; 
			return ret;
		}

		ret |= (byte & 0x7f) << shift;

		/* check for termination*/
		if ((byte & 0x80) == 0)
			return ret;

		/* advance to the next byte */
		mcp_consume(buf, 1);
		shift += 7;
	}
}

mcp_varlong_t mcp_varlong(struct mcp_parse *buf)
{
	mcp_varlong_t byte, ret = 0;
	const unsigned int num_bits = 64,
			max_shift = num_bits - 7,
			final_max = ALL_ONES(max_shift % 7);
	unsigned int shift = 0;

	/* pass errors */
	if (!mcp_ok(buf))
		return ret;

	while (1) {
		/* check if we are in bounds */
		if (mcp_avail(buf) <= 0) {
			buf->error = MCP_EOVERRUN;
			return ret;
		}

		byte = mcp_ptr(buf)[0];

		/* check if this would cause an overflow */
		if (shift > max_shift && byte > final_max) {
			buf->error = MCP_EOVERFLOW; 
			return ret;
		}

		ret |= (byte & 0x7f) << shift;

		/* check for termination*/
		if ((byte & 0x80) == 0)
			return ret;

		/* advance to the next byte */
		mcp_consume(buf, 1);
		shift += 7;
	}
}

mcp_svarint_t mcp_svarint(struct mcp_parse *buf)
{
	mcp_varint_t value = mcp_varint(buf);

	/* check the sign bit */
	if (value & 1)
		return -(mcp_svarint_t)(value >> 1);

	return value >> 1;
}

mcp_svarlong_t mcp_svarlong(struct mcp_parse *buf)
{
	mcp_varlong_t value = mcp_varlong(buf);

	/* check the sign bit */
	if (value & 1)
		return -(mcp_svarlong_t)(value >> 1);

	return value;
}

const void *mcp_bytes(struct mcp_parse *buf, size_t *size)
{
	assert(size);

	/* read the size prefix */
	*size = mcp_varint(buf);

	/* get the data pointer */
	return mcp_raw(buf, *size);
}

void mcp_copy_bytes(void *dest, struct mcp_parse *buf, size_t *max_size)
{
	size_t size;
	struct mcp_parse saved_buf = *buf;
	const void *value = mcp_bytes(buf, &size);

	assert(max_size);

	/* pass errors */
	if (!mcp_ok(buf))
		return;

	/* bounds check */
	if (size > *max_size) {
		*buf = saved_buf;
		return;
	}

	memcpy(dest, value, size);
	*max_size = size;
}

mcp_ubyte_t mcp_ubyte(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 1);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return value[0];
}

mcp_ushort_t mcp_ushort(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 2);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return ((mcp_ushort_t)value[0] << 8) | value[1];
}

mcp_uint_t mcp_uint(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 4);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return ((mcp_uint_t)value[0] << 24) | ((mcp_uint_t)value[1] << 16) |
			((mcp_uint_t)value[2] << 8) | value[3];
}

mcp_ulong_t mcp_ulong(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 8);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return ((mcp_ulong_t)value[0] << 56) | ((mcp_ulong_t)value[1] << 48) |
			((mcp_ulong_t)value[2] << 40) | ((mcp_ulong_t)value[3] << 32) |
			((mcp_ulong_t)value[4] << 24) | ((mcp_ulong_t)value[5] << 16) |
			((mcp_ulong_t)value[6] << 8) | value[7];
}

mcp_byte_t mcp_byte(struct mcp_parse *buf)
{
	mcp_ubyte_t value = mcp_ubyte(buf);
	
	/* check the sign bit */
	if (value & ONE_BIT(7))
		return -(value ^ ALL_ONES(8)) - 1;
	
	return value;
}

mcp_short_t mcp_short(struct mcp_parse *buf)
{
	mcp_ushort_t value = mcp_ushort(buf);
	
	/* check the sign bit */
	if (value & ONE_BIT(15))
		return -(value ^ ALL_ONES(16)) - 1;
	
	return value;
}

mcp_int_t mcp_int(struct mcp_parse *buf)
{
	mcp_uint_t value = mcp_uint(buf);
	
	/* check the sign bit */
	if (value & ONE_BIT(31))
		return -(value ^ ALL_ONES(32)) - 1;
	
	return value;
}

mcp_long_t mcp_long(struct mcp_parse *buf)
{
	mcp_ulong_t value = mcp_ulong(buf);
	
	/* check the sign bit */
	if (value & ONE_BIT(63))
		return -(value ^ ALL_ONES(64)) - 1;
	
	return value;
}

mcp_bool_t mcp_bool(struct mcp_parse *buf)
{
	return !!mcp_ubyte(buf);
}

float mcp_float(struct mcp_parse *buf)
{
#if MCP_SYSTEM_IEEE754_FLOAT
	union {
		mcp_uint_t i;
		float f;
	} value;
	assert(sizeof(mcp_uint_t) == sizeof(float));
	value.i = mcp_uint(buf);
	return value.f;
#else
	mcp_uint_t value = mcp_uint(buf);
	mcp_int_t significand = (1 << 23) | (value & ALL_ONES(23));
	int exponent = (value >> 23) & ALL_ONES(7);
	if (value & ONE_BIT(30))
		exponent = -exponent - 1;
	if (value & ONE_BIT(31))
		significand *= -1;
	return ldexp(significand, exponent - 23);
#endif
}

double mcp_double(struct mcp_parse *buf)
{
#if MCP_SYSTEM_IEEE754_FLOAT
	union {
		mcp_ulong_t i;
		double f;
	} value;
	assert(sizeof(mcp_ulong_t) == sizeof(double));
	value.i = mcp_ulong(buf);
	return value.f;
#else
	mcp_ulong_t value = mcp_ulong(buf);
	mcp_long_t significand = (1L << 52) | (value & ALL_ONES(52));
	int exponent = (value >> 23) & ALL_ONES(8);
	if (value & ONE_BIT(62))
		exponent = -exponent - 1;
	if (value & ONE_BIT(63))
		significand *= -1;
	return ldexp(significand, exponent - 52);
#endif
}
