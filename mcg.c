#include <stdlib.h>

#include "mcp_types.h"

#include "fbuf.h"
#include "mcp.h"

#include <assert.h>

/* returns an integer with a number of ones set starting from the lsb */
#define ALL_ONES(num_bits)      ((~(mcp_ulong_t)0) >>				\
									(8 * sizeof(mcp_ulong_t) - (num_bits)))
/* returns a long with bit number bit_num set, counting from the lsb */
#define ONE_BIT(bit_num)		(((mcp_ulong_t)1) << (bit_num))

#if !MCP_SYSTEM_IEEE754_FLOAT
# include <math.h>
#endif

/* for compatibility with varint28 */
#define MCG_BYTES_MAX_SIZE		(ALL_ONES(28))

int mcg_raw(struct fbuf *buf, const void *data, size_t size)
{
	/* copy the raw data into the buffer */
	if (fbuf_copy(buf, data, size) != size)
		return 1;
	return 0;
}

int mcg_varint(struct fbuf *buf, mcp_varint_t value)
{
	const int num_bits = 32,
			max_result_size = (num_bits + 6) / 7;
	unsigned char *dest = fbuf_wptr(buf, max_result_size);
	int i = 0;

#if USE_MCP_ASSERT_OVERFLOW
	assert(value <= ALL_ONES(num_bits));
#endif

	/* bounds check */
	if (value > ALL_ONES(num_bits))
		return 1;

	/* we could not allocate enough space. */
	if (dest == NULL)
		return 1;

	do {
		/* write out 7 bits */
		dest[i] = value & 0x7f;

		/* if the value is larger than 7 bits,
		 * then mark that there are more bytes to follow */
		if (value > 0x7f)
			dest[i] |= 0x80;

		/* advance to the next byte */
		i++;
		value >>= 7;

		/* if there are no more bits then stop*/
	} while(value > 0);
	return 0;
}

int mcg_varlong(struct fbuf *buf, mcp_varlong_t value)
{
	const int num_bits = 64,
			max_result_size = (num_bits + 6) / 7;
	unsigned char *dest = fbuf_wptr(buf, max_result_size);
	int i = 0;

#if USE_MCP_ASSERT_OVERFLOW
	assert(value <= ALL_ONES(num_bits));
#endif

	/* bounds check */
	if (value > ALL_ONES(num_bits))
		return 1;

	do {
		/* write out 7 bits */
		dest[i] = value & 0x7f;

		/* if the value is larger than 7 bits,
		 * then mark that there are more bytes to follow */
		if (value > 0x7f)
			dest[i] |= 0x80;

		/* advance to the next byte */
		i++;
		value >>= 7;

		/* if there are no more bits then stop*/
	} while(value > 0);
	return 0;
}

int mcg_svarint(struct fbuf *buf, mcp_svarint_t value)
{
	const int sig_bits = 31;
	const mcp_svarint_t max = ALL_ONES(sig_bits),
					min = -max - 1;
	mcp_varint_t packed = (mcp_varint_t)value << 2;

#if USE_MCP_ASSERT_OVERFLOW
	assert(value >= min && value <= max);
#endif

	/* overflow check */
	if (value < min || value > max)
		return 1;

	/* pack sign value */
	if (value < 0)
		value = (((mcp_varint_t)-(value + 1)) << 2) | 1;

	return mcg_varint(buf, packed);
}

int mcg_svarlong(struct fbuf *buf, mcp_svarlong_t value)
{
	const int sig_bits = 63;
	const mcp_svarlong_t max = ALL_ONES(sig_bits),
					min = -max - 1;
	mcp_varlong_t packed = (mcp_varint_t)value << 2;

#if USE_MCP_ASSERT_OVERFLOW
	assert(value >= min && value <= max);
#endif

	/* overflow check */
	if (value < min || value > max)
		return 1;

	/* pack sign value */
	if (value < 0)
		value = (((mcp_varlong_t)-(value + 1)) << 2) | 1;

	return mcg_varlong(buf, packed);
}

int mcg_bytes(struct fbuf *buf, const void *value, size_t size)
{
	/* overflow check */
	if (size > MCG_BYTES_MAX_SIZE)
		return 1;

	/* write size prefix */
	if (mcg_varint(buf, size))
		return 1;

	/* copy data */
	return mcg_raw(buf, value, size);
}

int mcg_ubyte(struct fbuf *buf, mcp_ubyte_t value)
{
	unsigned char data[1];

#if USE_MCP_ASSERT_OVERFLOW
	assert(value <= ALL_ONES(8));
#endif

	/* overflow check */
	if (value > ALL_ONES(8))
		return 1;

	/* write value */
	data[0] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_ushort(struct fbuf *buf, mcp_ushort_t value)
{
	unsigned char data[2];

#if USE_MCP_ASSERT_OVERFLOW
	assert(value <= ALL_ONES(16));
#endif

	/* overflow check */
	if (value > ALL_ONES(16))
		return 1;

	/* write value*/
	data[0] = (value >> 8) & 0xff;
	data[1] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_uint(struct fbuf *buf, mcp_uint_t value)
{
	unsigned char data[4];

#if USE_MCP_ASSERT_OVERFLOW
	assert(value <= ALL_ONES(32));
#endif

	/* overflow check */
	if (value > ALL_ONES(32))
		return 1;

	/* write value */
	data[0] = (value >> 24) & 0xff;
	data[1] = (value >> 16) & 0xff;
	data[2] = (value >> 8) & 0xff;
	data[3] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_ulong(struct fbuf *buf, mcp_ulong_t value)
{
	unsigned char data[8];

#if USE_MCP_ASSERT_OVERFLOW
	assert(value <= ALL_ONES(64));
#endif

	/* overflow check */
	if (value > ALL_ONES(64))
		return 1;

	/* write value */
	data[0] = (value >> 56) & 0xff;
	data[1] = (value >> 48) & 0xff;
	data[2] = (value >> 40) & 0xff;
	data[3] = (value >> 32) & 0xff;
	data[4] = (value >> 24) & 0xff;
	data[5] = (value >> 16) & 0xff;
	data[6] = (value >> 8) & 0xff;
	data[7] = value & 0xff;
	return mcg_raw(buf, data, sizeof(data));
}

int mcg_byte(struct fbuf *buf, mcp_byte_t value)
{
	const mcp_byte_t max = ALL_ONES(7),
					min = -max - 1;
#if !MCP_SYSTEM_TWOS_COMP
	const mcp_ubyte_t sign_bit = ONE_BIT(7);
#endif

#if USE_MCP_ASSERT_OVERFLOW
	assert(value >= min && value <= max);
#endif

	/* overflow check */
	if (value < min || value > max)
		return 1;

#if MCP_SYSTEM_TWOS_COMP
	return mcg_ubyte(buf, value);
#else
	/* pack two's complement */
	if (value < 0)
		return mcg_ubyte(buf, sign_bit | (mcp_ubyte_t)(sign_bit + value));

	return mcg_ubyte(buf, value);
#endif
}

int mcg_short(struct fbuf *buf, mcp_short_t value)
{
	const mcp_short_t max = ALL_ONES(15),
					min = -max - 1;
#if !MCP_SYSTEM_TWOS_COMP
	const mcp_ushort_t sign_bit = ONE_BIT(15);
#endif

#if USE_MCP_ASSERT_OVERFLOW
	assert(value >= min && value <= max);
#endif

	/* overflow check */
	if (value < min || value > max)
		return 1;

#if MCP_SYSTEM_TWOS_COMP
	return mcg_ushort(buf, value);
#else
	/* pack two's complement */
	if (value < 0)
		return mcg_ushort(buf, sign_bit | (mcp_ushort_t)(sign_bit + value));

	return mcg_ushort(buf, value);
#endif
}

int mcg_int(struct fbuf *buf, mcp_int_t value)
{
	const mcp_int_t max = ALL_ONES(31),
					min = -max - 1;
#if !MCP_SYSTEM_TWOS_COMP
	const mcp_uint_t sign_bit = ONE_BIT(31);
#endif

#if USE_MCP_ASSERT_OVERFLOW
	assert(value >= min && value <= max);
#endif

	/* overflow check */
	if (value < min || value > max)
		return 1;

#if MCP_SYSTEM_TWOS_COMP
	return mcg_uint(buf, value);
#else
	/* pack two's complement */
	if (value < 0)
		return mcg_uint(buf, sign_bit | (mcp_uint_t)(sign_bit + value));

	return mcg_uint(buf, value);
#endif
}

int mcg_long(struct fbuf *buf, mcp_long_t value)
{
	const mcp_long_t max = ALL_ONES(63),
					min = -max - 1;
#if !MCP_SYSTEM_TWOS_COMP
	const mcp_ulong_t sign_bit = ONE_BIT(63);
#endif

#if USE_MCP_ASSERT_OVERFLOW
	assert(value >= min && value <= max);
#endif

	/* overflow check */
	if (value < min || value > max)
		return 1;

#if MCP_SYSTEM_TWOS_COMP
	return mcg_ulong(buf, value);
#else
	/* pack two's complement */
	if (value < 0)
		return mcg_ulong(buf, sign_bit | (mcp_ulong_t)(sign_bit + value));

	return mcg_ulong(buf, value);
#endif
}

int mcg_bool(struct fbuf *buf, mcp_bool_t value)
{
	return mcg_ubyte(buf, !!value);
}

int mcg_float(struct fbuf *buf, float x)
{
#if USE_UNSAFE_MCP_FLOAT
	union {
		mcp_uint_float_t i;
		double f;
	} value;
	assert(sizeof(mcp_uint_float_t) == sizeof(float));
	/* WARNING: type-punning, your platform may not like this. */
	value.f = x;
	return mcg_uint(buf, value.i);
#else
	mcp_uint_t result = 0;
	int exp;
	double significand;
	if (x < 0) {
		x = -x;
		result |= ONE_BIT(31);
	}
	significand = frexp(x, &exp);
	exp -= 1;
	if (exp < 0)
		exp = -exp - 1;
	exp &= ALL_ONES(8);
	result |= exp << 23;
	result |= (mcp_uint_t)(significand * ONE_BIT(24)) & ALL_ONES(23);
	return mcg_uint(buf, result);
#endif
}

int mcg_double(struct fbuf *buf, double x)
{
#if MCP_SYSTEM_IEEE754_FLOAT
	union {
		mcp_uint_double_t i;
		double f;
	} value;
	assert(sizeof(mcp_uint_double_t) == sizeof(double));
	/* WARNING: type-punning, your platform may not like this. */
	value.f = x;
	return mcg_ulong(buf, value.i);
#else
	mcp_ulong_t result = 0;
	int exp;
	double significand;
	if (x < 0) {
		x = -x;
		result |= ONE_BIT(63);
	}
	significand = frexp(x, &exp);
	exp -= 1;
	if (exp < 0)
		exp = -exp - 1;
	exp &= ALL_ONES(11);
	result |= (mcp_ulong_t)exp << 52;
	result |= (mcp_uint_t)(significand * ONE_BIT(53)) & ALL_ONES(52);
	return mcg_ulong(buf, result);
#endif
}
