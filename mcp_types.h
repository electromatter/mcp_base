/* -*- PLATFORM CONFIG -*- */

/* Edit this file to define types that support the ranges:
 * ubyte 0 <= x <= 255
 * ushort 0 <= x <= 65535
 * uint 0 <= x <= 4294967296
 * ulong 0 <= x <= 18446744073709551616
 * byte: -128 <= x <= 127
 * short: -32769 <= x <= 32768
 * int: -2147483649 <= x <= 2147483648
 * long: -9223372036854775809 <= x <= 9223372036854775808
 * bool: 0 <= x <= 1
 * */

typedef char mcp_byte_t;
typedef short mcp_short_t;
typedef int mcp_int_t;
typedef long mcp_long_t;

typedef unsigned char mcp_ubyte_t;
typedef unsigned short mcp_ushort_t;
typedef unsigned int mcp_uint_t;
typedef unsigned long mcp_ulong_t;

typedef int mcp_bool_t;

/* thease types should be the exact same endianess and size
 * as IEEE754 floats and doubles respectively, if the system supports it */
typedef unsigned int mcp_uint_float_t;
typedef unsigned long mcp_uint_double_t;

/* -*- END OF PLATFORM CONFIG -*- */

/* -*- DEFAULT VALUES -*- */

typedef mcp_uint_t mcp_varint_t;
typedef mcp_ulong_t mcp_varlong_t;

typedef mcp_int_t mcp_svarint_t;
typedef mcp_long_t mcp_svarlong_t;

/* Enabling this causes mcg to use type-punning when interpreting floats */
#ifndef MCP_SYSTEM_IEEE754_FLOAT
# define MCP_SYSTEM_IEEE754_FLOAT			0
#endif

/* Enable this on a two's complement machine. It is faster. */
#ifndef MCP_SYSTEM_TWOS_COMP
# define MCP_SYSTEM_TWOS_COMP				0
#endif

/* This is to aid in debugging. Enabling this raises an assertion failure
 * when an invalid value is passed to mcg */
#ifndef USE_MCP_ASSERT_OVERFLOW
# define USE_MCP_ASSERT_OVERFLOW			0
#endif
