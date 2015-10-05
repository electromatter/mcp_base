/* mcp.c - Implementation of the mcp parse functions
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#include "mcp.h"

/* for memcpy */
#include <string.h>

/* for assert */
#include <assert.h>

/* asserts a valid buffer */
static inline void assert_valid_mcp(struct mcp_parse *buf)
{
	/* valid pointer */
	assert(buf);
	/* offset invariants */
	assert(buf->start <= buf->end);
	/* buffer is either empty, or non-empty and non-null*/
	assert(buf->base != NULL || (buf->base == NULL && buf->start == buf->end));
	/* error contains a valid error code */
	assert(buf->error == MCP_EOK || buf->error == MCP_EOVERRUN ||
			buf->error == MCP_EOVERFLOW);
}

/* returns the base pointer of the buffer */
static inline const unsigned char *mcp_ptr(struct mcp_parse *buf)
{   
	return buf->base + buf->start;
}

/* advances the offset; consuming a number of bytes */
static inline void mcp_consume(struct mcp_parse *buf, size_t size)
{   
	buf->start += size;
} 

const void *mcp_raw(struct mcp_parse *buf, size_t size)
{
	const void *ret;

	/* precondition */
	assert_valid_mcp(buf);

	/* pass errors */
	if (!mcp_ok(buf))
		return NULL;

	/* bounds check */
	if (mcp_avail(buf) < size) {
		buf->error = MCP_EOVERRUN;
		return NULL;
	}

	/* get the pointer to our raw data */
	ret = mcp_ptr(buf);

	/* update the pointers */
	mcp_consume(buf, size);
	return ret;
}

size_t mcp_copy_raw(void *dest, struct mcp_parse *buf, size_t size)
{
	/* get the pointer to our raw data and consume it */
	const void *value = mcp_raw(buf, size);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	/* copy it out */
	memcpy(dest, value, size);

	return size;
}

mcp_varint_t mcp_varint(struct mcp_parse *buf)
{
	mcp_varlong_t value = mcp_varlong(buf);

	/* check for overflow */
	if (value > UINT32_MAX) {
		buf->error = MCP_EOVERFLOW;
		return value;
	}

	return value;
}

mcp_varlong_t mcp_varlong(struct mcp_parse *buf)
{
	const unsigned char *base = mcp_ptr(buf);
	mcp_varlong_t ret = 0;
	int offset = 0;

	/* precondition */
	assert_valid_mcp(buf);

	/* pass errors */
	if (!mcp_ok(buf))
		return ret;

	do {
		/* check if we are in bounds */
		if ((size_t)offset >= mcp_avail(buf)) {
			buf->error = MCP_EOVERRUN;
			return ret;
		}

		/* check for overflow */
		if (offset == 9 && base[offset] > 0x1) {
			buf->error =  MCP_EOVERFLOW;
			return ret;
		}

		/* decode one byte */
		ret |= (mcp_varlong_t)(base[offset] & 0x7f) << (offset * 7);

		/* continue while the more-data-bit is set */
	} while (base[offset++] & 0x80);

	/* update the pointers */
	mcp_consume(buf, offset);
	return ret;
}

mcp_svarint_t mcp_svarint(struct mcp_parse *buf)
{
	mcp_varint_t value = mcp_varint(buf);

	/* check the sign bit */
	return (value & 1) ? ~(value >> 1) : (value >> 1);
}

mcp_svarlong_t mcp_svarlong(struct mcp_parse *buf)
{
	mcp_varlong_t value = mcp_varlong(buf);

	/* check the sign bit */
	return (value & 1) ? ~(value >> 1) : (value >> 1);
}

const void *mcp_bytes(struct mcp_parse *buf, size_t *size)
{
	mcp_varlong_t real_size = mcp_varlong(buf);
	assert(size);

	if (!mcp_ok(buf))
		return NULL;

	if (real_size > MCP_BYTES_MAX_SIZE) {
		buf->error = MCP_EOVERFLOW;
		return NULL;
	}

	/* read the size prefix */
	*size = real_size;

	/* get the data pointer */
	return mcp_raw(buf, *size);
}

size_t mcp_copy_bytes(void *dest, struct mcp_parse *buf, size_t max_size)
{
	size_t size;
	struct mcp_parse saved_buf = *buf;
	const void *value = mcp_bytes(buf, &size);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	/* bounds check */
	if (size > max_size) {
		/* roll-back the changes and return the actual size */
		*buf = saved_buf;
		return size;
	}

	/* copy out the bytes */
	memcpy(dest, value, size);

	/* return the actual size */
	return size;
}

size_t mcp_copy_string(char *dest, struct mcp_parse *buf, size_t max_size)
{
	size_t size;
	struct mcp_parse saved_buf = *buf;
	const void *value = mcp_bytes(buf, &size);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	/* bounds check */
	if (size > max_size) {
		/* roll-back the changes and return the actual size */
		*buf = saved_buf;
		return size + 1;
	}

	/* copy out the bytes */
	memcpy(dest, value, size);
	dest[size] = 0;

	/* return the actual size */
	return size + 1;
}

uint8_t mcp_ubyte(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 1);

	assert_valid_mcp(buf);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return value[0];
}

uint16_t mcp_ushort(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 2);

	assert_valid_mcp(buf);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return ((uint16_t)value[0] << 8) | value[1];
}

uint32_t mcp_uint(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 4);

	assert_valid_mcp(buf);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return ((uint32_t)value[0] << 24) | ((uint32_t)value[1] << 16) |
			((uint32_t)value[2] << 8) | value[3];
}

uint64_t mcp_ulong(struct mcp_parse *buf)
{
	const unsigned char *value = mcp_raw(buf, 8);

	assert_valid_mcp(buf);

	/* pass errors */
	if (!mcp_ok(buf))
		return 0;

	return ((uint64_t)value[0] << 56) | ((uint64_t)value[1] << 48) |
			((uint64_t)value[2] << 40) | ((uint64_t)value[3] << 32) |
			((uint64_t)value[4] << 24) | ((uint64_t)value[5] << 16) |
			((uint64_t)value[6] << 8) | value[7];
}

int8_t mcp_byte(struct mcp_parse *buf)
{
	return mcp_ubyte(buf);
}

int16_t mcp_short(struct mcp_parse *buf)
{
	return mcp_ushort(buf);
}

int32_t mcp_int(struct mcp_parse *buf)
{
	return mcp_uint(buf);
}

int64_t mcp_long(struct mcp_parse *buf)
{
	return mcp_ulong(buf);
}

int mcp_bool(struct mcp_parse *buf)
{
	return !!mcp_ubyte(buf);
}

float mcp_float(struct mcp_parse *buf)
{
	union {
		uint32_t i;
		float f;
	} value;

	/* check our assumption that double is type-punnable to uint32_t */
	assert(sizeof(uint32_t) == sizeof(float));

	/* read the float and type-pun it */
	value.i = mcp_uint(buf);

	return value.f;
}

double mcp_double(struct mcp_parse *buf)
{
	union {
		uint64_t i;
		double f;
	} value;

	/* check our assumption that double is type-punnable to uint64_t */
	assert(sizeof(uint64_t) == sizeof(double));

	/* read the double and type-pun it */
	value.i = mcp_ulong(buf);

	return value.f;
}
