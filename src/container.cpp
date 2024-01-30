#include "container.h"
#include "defaultTrace.h"
#include "defines/rethrowApi.h"
#include "err.h"
#include "processes.h"

#include <csignal>
#include <linux/sched.h> /* Definition of struct clone_args */
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

typedef struct
{
	uid_t uid;
	gid_t gid;
	void *data;
	err_t (*callback)(void *data);
} passToChild_t;

int createContainerCallback(void *data)
{
	err_t err = NO_ERRORCODE;
	passToChild_t *continerInfo = (passToChild_t *)data;
	QUITE_RETHROW(setUserPolicy(continerInfo->uid, continerInfo->gid));

	QUITE_CHECK(mount("proc", "/proc", "proc", 0, NULL) == 0);

	QUITE_RETHROW(continerInfo->callback(continerInfo->data));

cleanup:
	return err.errorCode;
}

THROWS err_t runInContainer(err_t (*callback)(void *), void *data, pid_t *childPid)
{
	err_t err = NO_ERRORCODE;
	char *stack;
	char *stackTop;
	pid_t pid;
	passToChild_t d = {.uid = getuid(), .gid = getgid(), .data = data, .callback = callback};

	CHECK(childPid != NULL);
	CHECK(*childPid <= 0);
	stack = (char *)mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	CHECK(stack != MAP_FAILED);
	stackTop = stack + STACK_SIZE;

	pid =
		clone(createContainerCallback, stackTop,
			  CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD | CLONE_PTRACE | CLONE_NEWCGROUP, &d);
	CHECK(pid != -1);

	*childPid = pid;

cleanup:
	return err;
}
