
#include "core.hpp"
#include "test.h"

#include <err.h>
#include <gtest/gtest.h>

#define CHECK_TIME(x)                                                                                                  \
	{                                                                                                                  \
		struct timespec start, end;                                                                                    \
		clock_gettime(CLOCK_REALTIME, &start);                                                                         \
		x;                                                                                                             \
		clock_gettime(CLOCK_REALTIME, &end);                                                                           \
		double f = ((double)end.tv_sec * 1e9 + end.tv_nsec) - ((double)start.tv_sec * 1e9 + start.tv_nsec);            \
		LOG_INFO("time {} ms", f / 1000000);                                                                         \
	}

int main(int argc, char **argv)
{
	err_t err = NO_ERRORCODE;
	int res = 0;

	std::cout << argv[1] << std::endl;
	::testing::InitGoogleTest(&argc, argv);

	QUITE_RETHROW(initLogger());

	CHECK_TIME(res = RUN_ALL_TESTS());

cleanup:
	QUITE_RETHROW(closeLogger());
	return res;
}

TEST(log, log)
{
	LOG_INFO("hello world");
}

TEST(log, logLevels)
{
	CHECK_TIME({
		LOG_TRACE("trace level");
		LOG_DEBUG("debug level");
		LOG_INFO("info level");
		LOG_WARN("warn level");
		LOG_ERR("err level");
		LOG_CRITICAL("critical level");
	});
}

TEST(log, logFromC)
{
	logFromC();
}

TEST(err, checkFail)
{
	err_t err = NO_ERRORCODE;
	CHECK(false);

cleanup:

	LOG_TRACE("error: {}", err.value);
}
