# mcp-base
---
A simple implemenation of fundimental types for building network
protocols.

### Assumptions
---
- 8-bit char
- Signed integers are represented using two's complement
- Floats are IEEE475 32 bit floats in host endian
- Doubles are IEEE 475 64 bit floats in host endian


### Examples
---
```c
struct mcp_parse buf = MCP_START_INITIALIZER(base, size);
mcp_varint_t length = mcp_varint(&buf);
if (!mcp_ok(&buf))
	/* There was an error! */
/* Otherwise, everything is fine. */
```

```c
struct fbuf buf = FBUF_INITIALIZER;
int ret = 0;
ret |= mcg_varint(buf, 123);
if (ret)
    /* There was an error! */
/* Otherwise, everything is fine. */
```

## API
---

