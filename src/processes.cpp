#include "processes.h"
#include "defaultTrace.h"
#include "err.h"
#include "files.h"

#include <fcntl.h>
#include <fmt/format.h>
#include <sys/wait.h>

#define PROCESS_NAME_PATH_FMT "/proc/%d/comm"
#define THREAD_NAME_PATH_FMT "/proc/%d/task/%d/comm"

THROWS err_t getProcessName(const pid_t pid, char *buf, int bufSize)
{
	err_t err = NO_ERRORCODE;
	std::string commPath = fmt::format(PROCESS_NAME_PATH_FMT, pid);
	ssize_t processNameSize = -1;
	fd_t fd = INVALID_FD;

	QUITE_CHECK(buf != NULL);
	QUITE_CHECK(bufSize >= 16);

	QUITE_RETHROW(safeOpenFmt(PROCESS_NAME_PATH_FMT, 0, O_RDONLY, &fd, pid));
	QUITE_RETHROW(safeRead(fd, buf, bufSize, &processNameSize));
	buf[processNameSize - 1] = '\0';

cleanup:
	if (IS_VALID_FD(fd))
	{
		safeClose(&fd);
	}

	return err;
}

THROWS err_t getThreadName(const pid_t pid, const pid_t tid, char *buf, int bufSize)
{
	err_t err = NO_ERRORCODE;
	ssize_t threadNameSize = -1;
	fd_t fd = INVALID_FD;

	QUITE_CHECK(buf != NULL);
	QUITE_CHECK(bufSize >= 16);

	QUITE_RETHROW(safeOpenFmt(THREAD_NAME_PATH_FMT, 0, O_RDONLY, &fd, pid, tid));
	QUITE_RETHROW(safeRead(fd, buf, bufSize, &threadNameSize));
	buf[threadNameSize - 1] = '\0';

cleanup:
	if (IS_VALID_FD(fd))
	{
		safeClose(&fd);
	}

	return err;
}

THROWS err_t safeWaitPid(const pid_t pid, processState_t *status, int options)
{
	err_t err = NO_ERRORCODE;
	int tempStatus;
	int res = 0;

	QUITE_CHECK(status != NULL);

	do
	{
		res = waitpid(pid, &tempStatus, options);
	} while (res >= 0 && errno == EINTR);

	QUITE_CHECK(res == pid || res == 0);

	status->exitBy.normal = WIFEXITED(tempStatus);
	status->exitBy.signal = WIFSIGNALED(tempStatus);
	status->exitBy.leftCoreDump = WCOREDUMP(tempStatus);
	status->exitBy.resumed = WIFCONTINUED(tempStatus);
	status->exitBy.stopped = WIFSTOPPED(tempStatus);

	if (status->exitBy.normal)
	{
		status->exitStatus = WEXITSTATUS(tempStatus);
	}
	else if (status->exitBy.signal)
	{

		status->terminatedBySignal = WTERMSIG(tempStatus);
	}
	else if (status->exitBy.stopped)
	{

		status->stopSignal = WSTOPSIG(tempStatus);
	}

cleanup:
	return err;
}

THROWS err_t setUserPolicy(const uid_t uid, const gid_t gid)
{
	err_t err = NO_ERRORCODE;
	ssize_t bytesWritten;
	fd_t uidMapFd = INVALID_FD;
	fd_t gidMapFd = INVALID_FD;
	fd_t setgroupsFd = INVALID_FD;

	QUITE_RETHROW(safeOpen("/proc/self/uid_map", O_WRONLY, 0, &uidMapFd));
	QUITE_RETHROW(safeOpen("/proc/self/setgroups", O_WRONLY, 0, &setgroupsFd));
	QUITE_RETHROW(safeOpen("/proc/self/gid_map", O_WRONLY, 0, &gidMapFd));

	QUITE_RETHROW(safeWriteFmt(uidMapFd, "0 %d 1", &bytesWritten, uid));
	QUITE_RETHROW(safeWrite(setgroupsFd, "deny", 5, &bytesWritten));
	QUITE_CHECK(bytesWritten == 5);

	QUITE_RETHROW(safeWriteFmt(gidMapFd, "0 %d 1", &bytesWritten, gid));

cleanup:

	if (IS_VALID_FD(uidMapFd))
	{
		safeClose(&uidMapFd);
	}

	if (IS_VALID_FD(gidMapFd))
	{
		safeClose(&gidMapFd);
	}

	if (IS_VALID_FD(setgroupsFd))
	{
		safeClose(&setgroupsFd);
	}

	return err;
}
