/* mcp_internal.h - internal functions for use with mcp_parse
 * use these when implementing new mcp_* functions
 *
 * Copyright (c) 2015 Eric Chai <electromatter@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the ISC license. See the LICENSE file for details.
 */

#ifndef MCP_INTERNAL
#define MCP_INTERNAL

#include "mcp.h"

/* for assert */
#include <assert.h>

/* asserts that buf is valid. You should call
 * this at the beginning of every mcp_* function */
static inline void assert_valid_mcp(struct mcp_parse *buf)
{
    /* valid pointer */
    assert(buf);
    /* offset invariants */
    assert(buf->start <= buf->end);
    /* buffer is either empty, or non-empty and non-null*/
    assert(buf->base != NULL || (buf->base == NULL && buf->start == buf->end));
}

/* returns the base pointer of the buffer. Use this
 * to access data in the buffer. */
static inline const unsigned char *mcp_ptr(struct mcp_parse *buf)
{   
    return buf->base + buf->start;
}

/* advances the offset; consuming a number of bytes */
static inline void mcp_consume(struct mcp_parse *buf, size_t size)
{   
    buf->start += size;
}

#endif

