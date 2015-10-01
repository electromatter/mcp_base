#include <stdlib.h>
#include "fbuf.h"

#include <stdio.h>

static int simple_test(void)
{
	struct fbuf buf = FBUF_INITIALIZER;
	fbuf_wptr(&buf, 1);
	fbuf_produce(&buf, 1);
	return 0;
}

static int random_test(void)
{
	return 0;
}

static int leak_test(void)
{
	return 0;
}

static int limit_test(void)
{
	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	(void)argc, (void)argv;

	ret = simple_test();
	if (ret)
		return ret;

	ret = random_test();
	if (ret)
		return ret;

	ret = leak_test();
	if (ret)
		return ret;

	ret = limit_test();
	if (ret)
		return ret;

	return 0;
}

