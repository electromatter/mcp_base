# mcp-base
---
A simple implemenation of fundimental types for building network
protocols.

Here is an example of mpc-base being used to parse a varint from a buffer:

```c
struct mcp_parse buf = MCP_START_INITIALIZER(base, size);
mcp_varint_t length = mcp_varint(&buf);
/* ... */
if (!mcp_ok(&buf))
	/* There was an error! */
/* Otherwise, everything is fine. */
```

Here is an example of using mcp-base to pack a varint:

```c
struct fbuf buf = FBUF_INITIALIZER;
int ret = 0;
ret |= mcg_varint(buf, 123);
/* ... */
if (ret)
    /* There was an error! */
/* Otherwise, everything is fine. */
```

#### TODO
---
api documetation... tests... verify mcp_float, mcg_float

splint/clang static analysis

valgrind tests

check rounding/nan/inf

remove limitations on varint

check grammer and spelling
