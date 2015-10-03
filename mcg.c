/* mcg.c - Implemenation mcp pack functions
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#include "fbuf.h"
#include "mcp.h"

#include <assert.h>
#include <string.h>

int mcg_raw(struct fbuf *buf, const void *data, size_t size)
{
	/* copy the raw data into the buffer */
	return fbuf_copy(buf, data, size);
}

int mcg_varint(struct fbuf *buf, mcp_varint_t value)
{
	unsigned char *dest = fbuf_wptr(buf, 5);
	int offset = 0;

	/* we could not allocate enough space. */
	if (dest == NULL)
		return 1;

	do {
		/* write out 7 bits */
		dest[offset] = value & 0x7f;

		/* if the value is larger than 7 bits,
		 * then mark that there are more bytes to follow */
		if (value > 0x7f)
			dest[offset] |= 0x80;

		/* advance to the next byte */
		offset++;
		value >>= 7;

		/* if there are no more bits then stop*/
	} while(value > 0);

	/*update pointers*/
	fbuf_produce(buf, offset);
	return 0;
}

int mcg_varlong(struct fbuf *buf, mcp_varlong_t value)
{
	unsigned char *dest = fbuf_wptr(buf, 10);
	int offset = 0;

	/* we could not allocate enough space. */
	if (dest == NULL)
		return 1;

	do {
		/* write out 7 bits */
		dest[offset] = value & 0x7f;

		/* if the value is larger than 7 bits,
		 * then mark that there are more bytes to follow */
		if (value > 0x7f)
			dest[offset] |= 0x80;

		/* advance to the next byte */
		offset++;
		value >>= 7;

		/* if there are no more bits then stop*/
	} while(value > 0);

	/*update pointers*/
	fbuf_produce(buf, offset);
	return 0;
}

int mcg_svarint(struct fbuf *buf, mcp_svarint_t value)
{
	return mcg_varint(buf, (value << 1) ^ (value >> 31));
}

int mcg_svarlong(struct fbuf *buf, mcp_svarlong_t value)
{
	return mcg_varlong(buf, (value << 1) ^ (value >> 63));
}

int mcg_bytes(struct fbuf *buf, const void *value, size_t size)
{
	/* overflow check */
	if (size > MCP_BYTES_MAX_SIZE)
		return 1;

	/* write size prefix */
	if (mcg_varlong(buf, size))
		return 1;

	/* copy data */
	return mcg_raw(buf, value, size);
}

int mcg_string(struct fbuf *buf, const char *value)
{
	return mcg_bytes(buf, value, strlen(value));
}

int mcg_ubyte(struct fbuf *buf, uint8_t value)
{
	unsigned char data[1];

	/* write value */
	data[0] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_ushort(struct fbuf *buf, uint16_t value)
{
	unsigned char data[2];

	/* write value*/
	data[0] = (value >> 8) & 0xff;
	data[1] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_uint(struct fbuf *buf, uint32_t value)
{
	unsigned char data[4];

	/* write value */
	data[0] = (value >> 24) & 0xff;
	data[1] = (value >> 16) & 0xff;
	data[2] = (value >> 8) & 0xff;
	data[3] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_ulong(struct fbuf *buf, uint64_t value)
{
	unsigned char data[8];

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

int mcg_byte(struct fbuf *buf, int8_t value)
{
	return mcg_ubyte(buf, value);
}

int mcg_short(struct fbuf *buf, int16_t value)
{
	return mcg_ushort(buf, value);
}

int mcg_int(struct fbuf *buf, int32_t value)
{
	return mcg_uint(buf, value);
}

int mcg_long(struct fbuf *buf, int64_t value)
{
	return mcg_ulong(buf, value);
}

int mcg_bool(struct fbuf *buf, int value)
{
	return mcg_ubyte(buf, !!value);
}

int mcg_float(struct fbuf *buf, float x)
{
	union {
		uint32_t i;
		float f;
	} value;

	/* verify our assumption that it is safe to type-pun float to uint32_t */
	assert(sizeof(uint32_t) == sizeof(float));

	/* type-pun float to uint32_t */
	value.f = x;

	/* write out the float */
	return mcg_uint(buf, value.i);
}

int mcg_double(struct fbuf *buf, double x)
{
	union {
		uint64_t i;
		double f;
	} value;

	/* verify our assumption that it is safe to type-pun double to uint64_t */
	assert(sizeof(uint64_t) == sizeof(double));

	/* type-pun double to uint64_t */
	value.f = x;

	/* write out the double */
	return mcg_ulong(buf, value.i);
}
