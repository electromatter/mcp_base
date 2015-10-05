# mcp_base
A simple library for parsing and packing fundimental types
for building network protocols. Licensed under the permissive ISC
license. See the LICENSE file for details. Comments and Improvements
are welcome.

NOTE: The tests supplied in tests/ do not exaustively test the contracts
of all of the functions, but they do check for many different errors.

## System Requirements
- C99 compiler
- stdlib: malloc, realloc, free, memmove, memcpy, strlen
- 8-bit char type
- Signed integers are represented using two's complement
- Floats are IEEE475 32 bit floats in host endian
- Doubles are IEEE 475 64 bit floats in host endian

## Example Usage
Parsing a structure containing a varint and a short:
```c
struct mcp_parse buf = MCP_START_INITIALIZER(base, size);
mcp_varint_t a = mcp_varint(&buf);
uint16_t b = mcp_ushort(&buf);
if (!mcp_ok(&buf))
	/* There was an error! */
/* Otherwise, everything is fine. */
```

Packing the same struct back into a fbuf:
```c
struct fbuf buf = FBUF_INITIALIZER;
int ret = 0;
ret |= mcg_varint(buf, a);
ret |= mcg_ushort(buf, b);
if (ret)
    /* There was an error! */
/* Otherwise, everything is fine. */
```

Readning from a unix socket into a fbuf:
```c
void *ptr = fbuf_wptr(&buf, BLOCK_SIZE);
if (ptr == NULL)
    /* Error: Buffer full. */
ssize_t ret = read(fd, ptr, fbuf_wavail(&buf));
if (ret <= 0)
    /* Error: see man read (2) */
fbuf_produce(&buf, ret);
/* report that we have read ret bytes from fd */
```

Writing to a unix socket from a fbuf:
```c
if (fbuf_avail(&buf) == 0)
    /* Error: No data in buffer to write. */
ssize_t ret = write(fd, fbuf_ptr(&buf), fbuf_avail(&buf));
if (ret < 0)
    /* Error: see man write (2) */
fbuf_consume(&buf, ret);
/* report that we have written ret bytes to fd */
```

## API Documentation
### fbuf.h

###### `FBUF_MAX`
The default and maximum value of max_size; the absolute maximum size of a fbuf.

###### `FBUF_INITALIZER`
Equivaliant to calling `fbuf_init` with `FBUF_MAX`.

###### `void fbuf_init(struct fbuf *buf, size_t max)`
Sets up an `buf` for first use. Limiting the maximum size 
of the buffer to `max` bytes.
Only call this function once per fbuf object.

###### `void fbuf_clear(struct fbuf *buf);`
Clears any data waiting in the `buf`, but keeps the memory block.

###### `void fbuf_free(struct fbuf *buf);`
Resets `buf` as if it were just initalized with `fbuf_init`.
Frees any block of memory owned by fbuf. Does not free `buf`, i.e. does not call `free(buf)`.

###### `const unsigned char *fbuf_ptr(struct fbuf *buf);`
Returns a pointer to the beginning of the data waiting in the buffer.
Use `fbuf_avail` to get the size of the block of data returned.

The pointers returned by `fbuf_wptr` and `fbuf_ptr` are invalidated by all `fbuf_` calls to the same fbuf object except `fbuf_wptr` with a `require` argument of zero, `fbuf_wavail`, `fbuf_ptr`, and `fbuf_avail`.

###### `size_t fbuf_avail(struct fbuf *buf);`
Returns the size of data waiting in the fbuf.

###### `unsigned char *fbuf_wptr(struct fbuf *buf, size_t require);`
Returns a pointer to use when writing data into `buf`. 
Expands the buffer to guarentee that the pointer returned
will be atleast `require` bytes long. If the memory allocation
failed, `fbuf_wptr` will return NULL.

The pointers returned by `fbuf_wptr` and `fbuf_ptr` are invalidated by all `fbuf_` calls to the same fbuf object except `fbuf_wptr` with a `require` argument of zero, `fbuf_wavail`, `fbuf_ptr`, and `fbuf_avail`.

###### `size_t fbuf_wavail(struct fbuf *buf);`
Returns the maximum size that wptr can satisfy without expanding
the `buf`.

###### `void fbuf_produce(struct fbuf *buf, size_t sz);`
Commits `sz` bytes to the buffer that have been written to the
pointer returned by the most recent call to `fbuf_wptr`.

###### `void fbuf_consume(struct fbuf *buf, size_t sz);`
Removed `sz` bytes from `buf` starting from the pointer returned by
the most recent call to `fbuf_ptr`.

###### `size_t fbuf_expand(struct fbuf *buf, size_t requested_size);`
Resizes `buf` so it can hold atleast `requested_size` more bytes in addition to the
data waiting in the buffer. `fbuf_expand` returns the the same value as `fbuf_wavail`

###### `void fbuf_compact(struct fbuf *buf);`
Rotates the buffer data so that the buffer starts at the data waiting in the buffer,
making future writes are more efficent. If you are doing
many repeated reads and writes, you should call this function to prevent the
buffer from much larger than the size of the data waiting in the buffer.

###### `int fbuf_shrink(struct fbuf *buf, size_t new_max);`
Changes the max size of the `buf` to new_max. Returns `0` if `fbuf_shrink`
succeedes without error, or `1` if new_max is too low and would truncate
data waiting in `buf`.

###### `int fbuf_copy(struct fbuf *dest, const void *src, size_t size);`
Copies `size` bytes from `src` into `dest`, expanding the buffer if necessary.
If the copy succeeds without error, `fbuf_copy` returns `0`.
Otherwise, it returns `1` on error.

### mcp.h

##### Fundimental Types
| mcp type    | c type                            | mcp type    | ctype                     |
|-------------|-----------------------------------|-------------|---------------------------|
| `raw`       | `void *`                          | `ubyte`     | `uint8_t`                 |
| `bytes`     | `void *` with `size_t`            | `short`     | `uint16_t`                |
| `string`    | `utf-8` `char *` `NUL`-terminated | `uint`      | `uint32_t`                |
| `bool`      | `int` as `1` or `0`               | `ulong`     | `uint64_t`                |
| `varint`    | `uint32_t`                        | `byte`      | `int8_t`                  |
| `varlong`   | `uint64_t`                        | `short`     | `int16_t`                 |
| `svarint`   | `int32_t`                         | `int`       | `int32_t`                 |
| `svarlong`  | `int64_t`                         | `long`      | `int16_t`                 |
| `float`     | `float`                           | `double`    | `double`                  |

###### `MCP_BYTES_MAX_SIZE`
The maximum accepted size of a bytes object. Useful for 
compatabilitiy with implementations that use varint28 as
a length prefix.

###### `MCP_EOK`
No error, the parser will continue to parse data from the buffer.

###### `MCP_EOVERRUN`
An overrun occured, there was not enough data in the buffer to
compleatly read the type requested. The parser is left in the
state as it was passed to the function that asserted this error.

###### `MCP_EOVERFLOW`
An overflow was caused by the data in the buffer. The parser is
left in the state as it was passed to the function that asserted
this error.

###### `int mcp_ok(struct mcp_parse *buf);`
Returns `1` if there are no errors asserted on `buf`. `0` otherwise.

###### `enum mcp_error_t mcp_error(struct mcp_parse *buf);`
Returns the error code asserted on `buf`.

###### `void mcp_start(struct mcp_parse *buf. const void *base, size_t size);`
Initalizes the parser, `buf`, for use with `size` bytes starting at `base`.

###### `int mcp_eof(struct mcp_parse *buf);`
Returns `1` if the parser has reached the end of the buffer. `0` otherwise.

###### `size_t mcp_avail(struct mcp_parse *buf);`
Returns the number of bytes waiting to be processed in `buf`.

###### `size_t mcp_copy_*type*(type *dest, struct mcp_parse *buf, size_t max_size);`
If `dest`, which is `max_size` bytes long, is large enough to
hold the object in `buf`, then `mcp_copy_*type*` copies the
object into `dest` and consumes it from `buf` and returns
the size in bytes of the resultant object in dest, if applicable,
including the `NUL`-terminator. If `dest` is too small to hold the object
`MCP_EOVERFLOW` is not asserted, and is not copied to `dest` and the
object is not consumed from `buf`. If there was an error decoding
the object from `buf` the error is asserted on `buf` and zero is returned and
the object is not consumed.

NOTE: For an object to be consumed, the parser simply advances the pointer past the object.

###### `*type* mcp_*type*(struct mcp_parse *buf);`
Decodes and and consumes returns the decoded object from `buf`. If there was
an error in decoding the object, an error is asserted on `buf`
and a place-holder value is returned (typically zero) and the object
is not consumed.

WARNING: Pointers returned by this function with the types `raw` and `bytes`
are only valid as long as base is valid. mcp_copy_*type* does not have this
restriction, but is slightly less efficent due to a copy.

NOTE: For an object to be consumed, the parser simply advances the pointer past the object.

###### `int mcg_*type*(struct fbuf *buf, *type* value);`
Packs an `value` into `buf`, expanding `buf` if necessary.
Returns `0` if successful and `1` if there was an error.
On error, partial values are not written. This is an
all-or-nothing function. However, if used as in the example 
above, `buf` may contain any combonation of failed and succeded
packed objects if `ret` is 1.

