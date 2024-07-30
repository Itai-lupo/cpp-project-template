#include "container.h"
#include "log.h"
#include "processes.h"
#include "test.h"

#include <fmt/color.h>
#include <fmt/core.h>

#include <pthread.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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

	pthread_setname_np(pthread_self(), fmt::format("logThread {}", a++).data());
	CHECK_TIME(for (int i = 0; i < 100000 / 1; i++) { LOG_TRACE("{}: {}", b, i); });

	return NULL;
}

using namespace fmt::literals;
err_t childMain([[maybe_unused]] void *data)
{

	err_t err = NO_ERRORCODE;
	[[maybe_unused]] int b = 0;
	pthread_t loggerThread[10] = {0};

	QUITE_CHECK(mount("./tmp", "./tmp", "tmpfs", 0, NULL) == 0);
	QUITE_RETHROW(initLogger());

	LOG_WARN("aa {} aa", fmt::styled(b, fmt::fg(fmt::color::green) | fmt::bg(fmt::color::blue)));
	LOG_ERR("aa");
	LOG_INFO("aa {aaa:}", "aaa"_a = gettid());
	logFromC();

	CHECK_TIME(
		for (size_t i = 0; i < 10; i++) { pthread_create(&loggerThread[i], NULL, benchmarck, NULL); }

		for (size_t i = 0; i < 10; i++) { pthread_join(loggerThread[i], NULL); });

cleanup:
	REWARN(closeLogger());
	return err;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	err_t err = NO_ERRORCODE;
	pid_t pid = 0;
	processState_t mainProcessExitStatus = {{{0, 0, 0, 0, 0}}, {0}};


	umask(0);
	QUITE_RETHROW(runInContainer(childMain, NULL, &pid));
	QUITE_RETHROW(safeWaitPid(pid, &mainProcessExitStatus, 0));

cleanup:
	if (mainProcessExitStatus.exitBy.normal)
	{
		printf("exited normaly with %d\n", mainProcessExitStatus.exitStatus);
		return mainProcessExitStatus.exitStatus;
	}
	else if (mainProcessExitStatus.exitBy.signal)
	{
		printf("exited by signal %d and %s left coredump ", mainProcessExitStatus.terminatedBySignal,
			   (mainProcessExitStatus.exitBy.leftCoreDump ? "" : "didn't"));
	}

	return err.errorCode;
}
