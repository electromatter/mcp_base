#include <stdlib.h>
#include "fbuf.h"

#include <string.h>
#include <assert.h>

/* fbuf valid object assertion
 * 1. (base and size are NULL) or (base and size are not NULL)
 * 2. start < size and end <= size and start <= end
 * 3. start = end = 0 or start < end
 * 4. size <= max
 * */
#define fbuf_assert(buf)												\
			assert((buf) &&												\
					((buf)->base != NULL) == ((buf)->size > 0) &&		\
					(buf)->end <= (buf)->size &&						\
					((buf)->start < (buf)->end ||						\
						((buf)->start == 0 && (buf)->end == 0)) &&		\
					((buf)->size <= (buf)->max_size))

#define FBUF_INITIAL_SIZE	(1024)
#define FBUF_EXPAND_COEFF	(2)

void fbuf_free(struct fbuf *buf)
{
	fbuf_assert(buf);

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
	fbuf_assert(buf);

	/* overflow check */
	assert(sz <= buf->size - buf->end);

	buf->end += sz;
}

void fbuf_consume(struct fbuf *buf, size_t sz)
{
	fbuf_assert(buf);

	/* overflow check */
	assert(sz <= fbuf_avail(buf));

	buf->start += sz;

	/* if our buffer is empty, clear it */
	if (fbuf_avail(buf) == 0)
		fbuf_clear(buf);
}

static size_t next_size(size_t size, size_t required_size, size_t max_size)
{
	if (size == 0)
		size = FBUF_INITIAL_SIZE;

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
	fbuf_assert(buf);

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
	new_size = next_size(buf->size, requested_size, buf->max_size);
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
	fbuf_assert(buf);

	/* rotate the buffer so that base points to the begining */
	memmove(buf->base, fbuf_ptr(buf), fbuf_avail(buf));

	/* update the pointers */
	buf->end -= buf->start;
	buf->start = 0;
}

size_t fbuf_copy(struct fbuf *dest, const void *src, size_t size)
{
	void *ptr = fbuf_wptr(dest, size);

	/* check that we can actually write this block */
	if (ptr == NULL)
		return 0;

	/* copy the data into the buffer */
	memcpy(ptr, src, size);

	/* and commit it */
	fbuf_produce(dest, size);
	return size;
}

