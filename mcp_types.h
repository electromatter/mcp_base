/* this file is customized for my compiler on my system, you should edit
 * it to fit your system 
 *
 * varint must be 32 bits
 * varlong must be 64 bits
 *
 * Value ranges:
 * byte: -128 <= x <= 127
 * short: -65536 <= x <= 65535
 * int should be atleast 32 bits
 * long should be atleast 64 bits
 * */

/* enable if float and double are the alias-able to mcp_int_t and mcp_long_t */
/* #define USE_UNSAFE_MCP_FLOAT		1 */
/* enable if the system is twos complement */
/* #define USE_MCP_TWOS_COMP		1 */

/* signed */
typedef int mcp_svarint_t;
typedef long mcp_svarlong_t;

typedef char mcp_byte_t;
typedef short mcp_short_t;
typedef int mcp_int_t;
typedef long mcp_long_t;

/* unsigned */
typedef unsigned int mcp_varint_t;
typedef unsigned long mcp_varlong_t;

typedef unsigned char mcp_ubyte_t;
typedef unsigned short mcp_ushort_t;
typedef unsigned int mcp_uint_t;
typedef unsigned long mcp_ulong_t;

typedef int mcp_bool_t;

