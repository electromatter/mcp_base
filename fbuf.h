/* fbuf.h
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#ifndef FBUF_H
#define FBUF_H

/* for size_t */
#include <stdlib.h>

struct fbuf {
	/* base pointer */
	unsigned char *base;
	/* size of the buffer and start/end of valid data */
	size_t size, max_size, start, end;
};

/* FBUF_MAX is the maximum value of max_size */
#define FBUF_MAX				((~(size_t)0) >> 1)

/* use fbuf_init to setup the buffer for first use */
#define FBUF_INITIALIZER		{NULL, 0, FBUF_MAX, 0, 0}
static inline void fbuf_init(struct fbuf *buf, size_t max)
{
	buf->base = NULL;
	buf->size = 0;
	buf->max_size = max;
	buf->start = 0;
	buf->end = 0;
}

/* clear the contents of the buffer, but keep the memory block */
static inline void fbuf_clear(struct fbuf *buf)
{
	buf->start = 0;
	buf->end = 0;
}


/* frees any blocks of memory owned by fbuf and resets the
 * buffer so it can be reused as if it were just initialized with fbuf_init */
void fbuf_free(struct fbuf *buf);

/* get a pointer to the available data to read */
static inline const unsigned char *fbuf_ptr(struct fbuf *buf)
{
	return buf->base + buf->start;
}

/* get the size of the data waiting to be read */
static inline size_t fbuf_avail(struct fbuf *buf)
{
	return buf->end - buf->start;
}

/* returns a pointer to the write end of the buffer with atleast require bytes
 * available to write to.
 * or returns null if the we fail to allocate enough space */
unsigned char *fbuf_wptr(struct fbuf *buf, size_t require);

/* get the size of the available space for writing */
static inline size_t fbuf_wavail(struct fbuf *buf)
{
	return buf->size - buf->end;
}

/* advances the write pointer; produces data */
void fbuf_produce(struct fbuf *buf, size_t sz);
/* rolls back sz bytes from the write end */
void fbuf_unproduce(struct fbuf *buf, size_t sz);
/* advances the read pointer; consumes data */
void fbuf_consume(struct fbuf *buf, size_t sz);

/* expands the buffer so that it can hold at least requested_size more bytes
 * returns the size of the writeable space */
size_t fbuf_expand(struct fbuf *buf, size_t requested_size);
/* rotates the buffer so that the read pointer is at the begining. */
void fbuf_compact(struct fbuf *buf);
/* resizes the buffer, if possible and changes the maximum size
 * If the operation succeeds, fbuf_shrink returns 0
 * Otherwise, it returns 1
 * fbuf_shrink may fail if there is no memory available,
 * or if new_max < fbuf_avail(buf)
 * If fbuf_shrinks, buf is not modified. */
int fbuf_shrink(struct fbuf *buf, size_t new_max);

/* copies data into the buffer
 * returns one if there was not enough space */
int fbuf_copy(struct fbuf *dest, const void *src, size_t size);

#endif
