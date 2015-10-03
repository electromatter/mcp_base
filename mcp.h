/* mcp.h
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#ifndef MCP_H
#define MCP_H

/* for size_t */
#include <stdlib.h>

/* for (u)int{8,16,32,64}_t*/
#include <stdint.h>

/* for compatibility with varint28 */
#ifndef MCP_BYTES_MAX_SIZE
# define MCP_BYTES_MAX_SIZE			(268435455)
#endif

enum mcp_error {
	MCP_EOK							= 0,
	MCP_EOVERRUN					= 1,
	MCP_EOVERFLOW					= 2
};

struct mcp_parse {
	/* source buffer */
	const unsigned char *base;
	/* the start/end of the data */
	size_t start, end;
	/* error code on this buffer, zero if none */
	enum mcp_error error;
};

struct fbuf;

typedef uint32_t mcp_varint_t;
typedef uint64_t mcp_varlong_t;
typedef int32_t mcp_svarint_t;
typedef int64_t mcp_svarlong_t;

/* returns true if there are no errors */
static inline int mcp_ok(struct mcp_parse *buf)
{
	return buf->error != MCP_EOK;
}

/* returns the error */
static inline enum mcp_error mcp_error(struct mcp_parse *buf)
{
	return buf->error;
}

/* initialzes a mcp_parse structure */
static inline void mcp_start(struct mcp_parse *buf, const void *base, size_t size)
{
	buf->base = base;
	buf->start = 0;
	buf->end = size;
	buf->error = MCP_EOK;
}

/* returns true if we have reached the end of the buffer */
static inline int mcp_eof(struct mcp_parse *buf)
{
	return buf->start >= buf->end;
}

/* returns the number of bytes that we can access from mcp_ptr */
static inline size_t mcp_avail(struct mcp_parse *buf)
{
	return buf->end - buf->start;
}
  
/* parse functions consumes data from buf and returns the parsed value
 *
 * pointer values returned from these functions are zero-copy
 * meaning they are scoped to the scope of the base pointer of
 * the respective mcp_parse context
 *
 * use mcp_copy_* to copy the value
 *
 * If there was an error when parsing data, the error field is set in the
 * context.
 *
 * use mcp_ok to check if the context has no errors and
 * mcp_error to get more information about the specific error
 *
 * it is safe to continue calling these functions with a context that has
 * an error. calls after the error field is set will return a dummy value.
 */
const void *mcp_raw(struct mcp_parse *buf, size_t size);
void mcp_copy_raw(void *dest, struct mcp_parse *buf, size_t size);

mcp_varint_t mcp_varint(struct mcp_parse *buf);
mcp_varlong_t mcp_varlong(struct mcp_parse *buf);
mcp_svarint_t mcp_svarint(struct mcp_parse *buf);
mcp_svarlong_t mcp_svarlong(struct mcp_parse *buf);

const void *mcp_bytes(struct mcp_parse *buf, size_t *size);
/* if dest is not large enough to hold the bytes, max_size is set to the
 * size of the bytes object, but buf is not touched, resize dest to hold
 * atleast max_size bytes and call this function again to copy the data */
size_t mcp_copy_bytes(void *dest, struct mcp_parse *buf, size_t max_size);

uint8_t mcp_ubyte(struct mcp_parse *buf);
uint16_t mcp_ushort(struct mcp_parse *buf);
uint32_t mcp_uint(struct mcp_parse *buf);
uint64_t mcp_ulong(struct mcp_parse *buf);

int8_t mcp_byte(struct mcp_parse *buf);
int16_t mcp_short(struct mcp_parse *buf);
int32_t mcp_int(struct mcp_parse *buf);
int64_t mcp_long(struct mcp_parse *buf);

int mcp_bool(struct mcp_parse *buf);

float mcp_float(struct mcp_parse *buf);
double mcp_double(struct mcp_parse *buf);

/* generator functions take a value and produce data
 * these functions return zero if there was no error
 *
 * this can be used to simplify generator error-handling code:
 * int ret = 0;
 * ret |= mcg_varint(buf, 14);
 * ...
 * if (ret) {
 *	// handle error
 * }
 * // all ok
 */
int mcg_raw(struct fbuf *buf, const void *data, size_t size);

int mcg_varint(struct fbuf *buf, mcp_varint_t value);
int mcg_varlong(struct fbuf *buf, mcp_varlong_t value);
int mcg_svarint(struct fbuf *buf, mcp_svarint_t value);
int mcg_svarlong(struct fbuf *buf, mcp_svarlong_t value);

int mcg_bytes(struct fbuf *buf, const void *value, size_t size);
int mcg_string(struct fbuf *buf, const char *value);

int mcg_ubyte(struct fbuf *buf, uint8_t value);
int mcg_ushort(struct fbuf *buf, uint16_t value);
int mcg_uint(struct fbuf *buf, uint32_t value);
int mcg_ulong(struct fbuf *buf, uint64_t value);

int mcg_byte(struct fbuf *buf, int8_t value);
int mcg_short(struct fbuf *buf, int16_t value);
int mcg_int(struct fbuf *buf, int32_t value);
int mcg_long(struct fbuf *buf, int64_t value);

int mcg_bool(struct fbuf *buf, int value);

int mcg_float(struct fbuf *buf, float value);
int mcg_double(struct fbuf *buf, double value);

#endif
