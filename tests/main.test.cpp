
#include "container.h"
#include "core.hpp"
#include "defines/logMacros.h"
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
		LOG_INFO("time {} ms", f / 1000000);                                                                           \
	}

err_t testsMain([[maybe_unused]] void *data)
{
	err_t err = NO_ERRORCODE;
	int res = 0;
	RETHROW(initLogger());
	CHECK_TIME(res = RUN_ALL_TESTS());
	CHECK(res == 0);

cleanup:
	REWARN(closeLogger());
	return err;
}

int main(int argc, char **argv)
{
	err_t err = NO_ERRORCODE;
	pid_t pid = -1;

	::testing::InitGoogleTest(&argc, argv);
	RETHROW(runInContainer(testsMain, 0, &pid));
	QUITE_CHECK(waitpid(pid, NULL, 0) == pid);

cleanup:
	return err.errorCode;
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
	ASSERT_FALSE(true);
	LOG_TRACE("error: {}", err.value);
}
