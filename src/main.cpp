#include "allocators/sharedMemoryPool.h"
#include "container.h"
#include "defines/rethrowApi.h"
#include "log.h"
#include "processes.h"

#include <fmt/color.h>
#include <fmt/core.h>

#include <pthread.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "types/err_t.h"

#define CHECK_TIME(x)                                                                                                  \
	{                                                                                                                  \
		struct timespec start, end;                                                                                    \
		clock_gettime(CLOCK_REALTIME, &start);                                                                         \
		x;                                                                                                             \
		clock_gettime(CLOCK_REALTIME, &end);                                                                           \
		double f = ((double)end.tv_sec * 1e9 + end.tv_nsec) - ((double)start.tv_sec * 1e9 + start.tv_nsec);            \
		LOG_INFO("time {} ms", f / 1000000);                                                                           \
	}

void *benchmarck(void *)
{
	static int a = 0;
	int b = a;
	err_t err = NO_ERRORCODE;
	pthread_setname_np(pthread_self(), fmt::format("logThread {}", a++).data());
	CHECK_TIME(for (int i = 0; i < 100000 / 1; i++) { LOG_TRACE("{}: {}", b, i); });

	return NULL;
}

err_t childMain([[maybe_unused]] void *data)
{

	err_t err = NO_ERRORCODE;
	[[maybe_unused]] int b = 0;
	pthread_t logger_thread[10] = {0};

	QUITE_CHECK(mount("./tmp", "./tmp", "tmpfs", 0, NULL) == 0);
	QUITE_RETHROW(initSharedMemory());
	QUITE_RETHROW(initLogger());

	CHECK_TIME(
		for (size_t i = 0; i < 10; i++) { pthread_create(&logger_thread[i], NULL, benchmarck, NULL); }

		for (size_t i = 0; i < 10; i++) { pthread_join(logger_thread[i], NULL); });

cleanup:
	QUITE_RETHROW(closeSharedMemory());

	REWARN(closeLogger());
	return err;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	err_t err = NO_ERRORCODE;
	pid_t pid = 0;
	processState_t main_process_exit_status = {{{0, 0, 0, 0, 0}}, {0}};

	umask(0);
	QUITE_RETHROW(runInContainer(childMain, NULL, &pid));
	QUITE_RETHROW(safeWaitPid(pid, &main_process_exit_status, 0));

cleanup:
	if (main_process_exit_status.exitBy.normal)
	{
		printf("exited normaly with %d\n", main_process_exit_status.exitStatus);
		return main_process_exit_status.exitStatus;
	}
	else if (main_process_exit_status.exitBy.signal)
	{
		printf("exited by signal %d and %s left coredump ", main_process_exit_status.terminatedBySignal,
			   (main_process_exit_status.exitBy.leftCoreDump ? "" : "didn't"));
	}

	return err.errorCode;
}
