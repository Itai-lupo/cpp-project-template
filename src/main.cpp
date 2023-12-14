#include "core.hpp"
#include "test.h"
#include <cstdio>
#include <ctime>
#include <err.h>
#include <pthread.h>
#include <unistd.h>

void *benchmarck(void *)
{
	struct timespec threadStartTime;
	struct timespec processStartTime;

	struct timespec threadEndTime;
	struct timespec processEndTime;

	struct timespec threadDiffTime;
	struct timespec processDiffTime;
	static int a = 0;
	pthread_setname_np(pthread_self(), fmt::format("logThread {}", a++).data());
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &threadStartTime);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &processStartTime);

	for (int i = 0; i < 100000; i++)
	{

		LOG_TRACE("start time = {}, {}", threadStartTime.tv_nsec, i);
	}

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &threadEndTime);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &processEndTime);

	threadDiffTime = {threadEndTime.tv_sec - threadStartTime.tv_sec, threadEndTime.tv_nsec - threadStartTime.tv_nsec};
	processDiffTime = {processEndTime.tv_sec - processStartTime.tv_sec,
					   processEndTime.tv_nsec - processStartTime.tv_nsec};

	LOG_INFO("time on benchmarck thread {}.{}", threadDiffTime.tv_sec, threadDiffTime.tv_nsec);
	LOG_INFO("time on for full process {}.{}", processDiffTime.tv_sec, processDiffTime.tv_nsec);

	return NULL;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	err_t err = NO_ERRORCODE;
	[[maybe_unused]] int b = 0;
	struct timespec startTime;
	struct timespec endTime;
	struct timespec diffTime;
	pthread_t loggerThread[10] = {0};

	LOG_WARN("aa {} aa", b);
	LOG_ERR("aa");
	LOG_INFO("aa {}", gettid());
	logFromC();

	clock_gettime(CLOCK_REALTIME, &startTime);

	for (size_t i = 0; i < 10; i++)
	{
		pthread_create(&loggerThread[i], NULL, benchmarck, NULL);
	}

	for (size_t i = 0; i < 10; i++)
	{
		pthread_join(loggerThread[i], NULL);
	}

	clock_gettime(CLOCK_REALTIME, &endTime);

	diffTime = {endTime.tv_sec - startTime.tv_sec, endTime.tv_nsec - startTime.tv_nsec};
	LOG_TRACE("time diff on main thread {}.{}", diffTime.tv_sec, diffTime.tv_nsec);
	LOG_DEBUG("time diff on main thread {}.{}", diffTime.tv_sec, diffTime.tv_nsec);
	LOG_INFO("time diff on main thread {}.{}", diffTime.tv_sec, diffTime.tv_nsec);
	LOG_WARN("time diff on main thread {}.{}", diffTime.tv_sec, diffTime.tv_nsec);
	LOG_ERR("time diff on main thread {}.{}", diffTime.tv_sec, diffTime.tv_nsec);
	LOG_CRITICAL("time diff on main thread {}.{}", diffTime.tv_sec, diffTime.tv_nsec);

	return err.value;
}
