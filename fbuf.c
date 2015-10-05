/* fbuf.c
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#include <stdlib.h>
#include "fbuf.h"

#include <string.h>
#include <assert.h>

/* verify fbuf invariants */
static inline void assert_valid_fbuf(struct fbuf *buf)
{
	/* valid pointer */
	assert(buf);
	/* base = NULL <=> buf->size = 0*/
	assert((buf->base != NULL) == (buf->size > 0));
	/* start < size and end <= size and start <= end */
	assert(buf->end <= buf->size);
	/* start = end = 0 or start < end */
	assert(buf->start < buf->end || (buf->start == 0 && buf->end == 0));
	/* limit invariant */
	assert(buf->size <= buf->max_size);
}

#define FBUF_INITIAL_SIZE	(1024)
#define FBUF_EXPAND_COEFF	(2)

void fbuf_free(struct fbuf *buf)
{
	assert_valid_fbuf(buf);

	/* if we have a non-zero object then free it's buffer */
	if (buf->base)
		free(buf->base);

	/* and ensure it is cleared */
	buf->base = 0;
	buf->size = 0;

	fbuf_clear(buf);	
}

unsigned char *fbuf_wptr(struct fbuf *buf, size_t require)
{
	/* check if we need to expand, then if expand failed return null */
	if (fbuf_wavail(buf) < require && fbuf_expand(buf, require) < require)
		return NULL;

	return buf->base + buf->end;
}

void fbuf_produce(struct fbuf *buf, size_t sz)
{
	assert_valid_fbuf(buf);

	/* overflow check */
	assert(sz <= fbuf_wavail(buf));

	buf->end += sz;
}

void fbuf_unproduce(struct fbuf *buf, size_t sz)
{
	assert_valid_fbuf(buf);

	/* underflow check */
	assert(sz <= fbuf_avail(buf));

	buf->end -= sz;
}

void fbuf_consume(struct fbuf *buf, size_t sz)
{
	assert_valid_fbuf(buf);

	/* overflow check */
	assert(sz <= fbuf_avail(buf));

	buf->start += sz;

	/* if our buffer is empty, clear it */
	if (fbuf_avail(buf) == 0)
		fbuf_clear(buf);
}

static size_t next_size(size_t required_size, size_t max_size)
{
	size_t size = FBUF_INITIAL_SIZE;

	/* exponentialy expand, with overflow check */
	while (size < required_size && size < FBUF_MAX / FBUF_EXPAND_COEFF)
		size *= FBUF_EXPAND_COEFF;

	/* overflow! clamp to max value */
	if (size < required_size)
		size = FBUF_MAX;

	/* clamp to max value */
	if (size > max_size)
		size = max_size;

	return size;
}

size_t fbuf_expand(struct fbuf *buf, size_t requested_size)
{
	size_t new_size;
	unsigned char *new_base;
	assert_valid_fbuf(buf);

	/* check if we can already satisfy this request */
	if (fbuf_wavail(buf) >= requested_size)
		return fbuf_wavail(buf);

	/* compute the required size of the buffer */
	requested_size += fbuf_avail(buf);

	/* check if we can ever satisfy this request */
	if (buf->max_size < requested_size)
		return fbuf_wavail(buf);

	/* check if we can just compact the buffer to satisfy the request */
	if (buf->size >= requested_size) {
		fbuf_compact(buf);
		return fbuf_wavail(buf);
	}

	/* allocate the new buffer */
	new_size = next_size(requested_size, buf->max_size);
	new_base = malloc(new_size);

	/* check if malloc failed*/
	if (new_base == NULL)
		return fbuf_wavail(buf);

	/* compact the buffer into the new buffer */
	memcpy(new_base, fbuf_ptr(buf), fbuf_avail(buf));

	if (buf->base)
		free(buf->base);

	/* update the pointers*/
	buf->base = new_base;
	buf->size = new_size;
	buf->end -= buf->start;
	buf->start = 0;

	return fbuf_wavail(buf);
}

void fbuf_compact(struct fbuf *buf)
{
	assert_valid_fbuf(buf);

	/* rotate the buffer so that base points to the begining */
	memmove(buf->base, fbuf_ptr(buf), fbuf_avail(buf));

	/* update the pointers */
	buf->end -= buf->start;
	buf->start = 0;
}

int fbuf_shrink(struct fbuf *buf, size_t new_max)
{
	void *new_base;
	assert_valid_fbuf(buf);

	/* check if new_max can hold the data currently in the buffer */
	if (fbuf_avail(buf) > new_max)
		return 1;

	/* check if we need to resize the buffer */
	if (buf->max_size <= new_max || buf->size <= new_max) {
		buf->max_size = new_max;
		return 0;
	}
	
	/* avoid calling realloc with size=0 */
	if (new_max == 0) {
		free(buf->base);
		buf->base = NULL;
		buf->size = 0;
		buf->max_size = 0;
		return 0;
	}

	/* compact and realloc */
	fbuf_compact(buf);
	new_base = realloc(buf->base, new_max);

	/* check that realloc succeeded */
	if (new_base != NULL)
		buf->base = new_base;

	/* update pointers */
	buf->size = new_max;
	buf->max_size = new_max;
	return 0;
}

int fbuf_copy(struct fbuf *dest, const void *src, size_t size)
{
	void *ptr = fbuf_wptr(dest, size);

	/* check that we can actually write this block */
	if (ptr == NULL)
		return 1;

	/* copy the data into the buffer */
	memcpy(ptr, src, size);

	/* and commit it */
	fbuf_produce(dest, size);
	return 0;
}
