/* USAGE:

// for size_t
#include <stdlib.h>

// for the fundimental types
#include "mcp_types.h"

#include "fbuf.h"
#include "mcp.h"
*/

enum mcp_error {
	MCP_EOK			= 0,
	MCP_EOVERRUN	= 1,
	MCP_EOVERFLOW	= 2
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

/* returns true if there are no errors */
#define mcp_ok(buf)						(!(buf)->error)

/* returns the error */
#define mcp_error(buf)					((buf)->error)

/* initialzes a mcp_parse structure */
#define mcp_start(buf, base, size)		do {(buf)->base = base;				\
										(buf)->start = 0; (buf)->end = size;\
										(buf)->error = MCP_EOK;} while (0)
#define mcp_start_fbuf(buf, fbuf)		mcp_start((buf),					\
											fbuf_ptr((fbuf)),				\
											fbuf_avail((fbuf)))

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
void mcp_copy_bytes(void *dest, struct mcp_parse *buf, size_t *max_size);

mcp_ubyte_t mcp_ubyte(struct mcp_parse *buf);
mcp_ushort_t mcp_ushort(struct mcp_parse *buf);
mcp_uint_t mcp_uint(struct mcp_parse *buf);
mcp_ulong_t mcp_ulong(struct mcp_parse *buf);

mcp_byte_t mcp_byte(struct mcp_parse *buf);
mcp_short_t mcp_short(struct mcp_parse *buf);
mcp_int_t mcp_int(struct mcp_parse *buf);
mcp_long_t mcp_long(struct mcp_parse *buf);

mcp_bool_t mcp_bool(struct mcp_parse *buf);

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

int mcg_ubyte(struct fbuf *buf, mcp_ubyte_t value);
int mcg_ushort(struct fbuf *buf, mcp_ushort_t value);
int mcg_uint(struct fbuf *buf, mcp_uint_t value);
int mcg_ulong(struct fbuf *buf, mcp_ulong_t value);

int mcg_byte(struct fbuf *buf, mcp_byte_t value);
int mcg_short(struct fbuf *buf, mcp_short_t value);
int mcg_int(struct fbuf *buf, mcp_int_t value);
int mcg_long(struct fbuf *buf, mcp_long_t value);

int mcg_bool(struct fbuf *buf, mcp_bool_t value);

int mcg_float(struct fbuf *buf, float value);
int mcg_double(struct fbuf *buf, double value);

