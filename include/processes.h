#pragma once

#include "types/err_t.h"

#include <bits/types/struct_timespec.h>
#include <cstdint>
#include <sched.h>
#include <unistd.h>

typedef struct
{
	union {

		struct
		{
			uint8_t normal : 1;
			uint8_t signal : 1;
			uint8_t leftCoreDump : 1;
			uint8_t stopped : 1;
			uint8_t resumed : 1;
		};
		uint8_t value;
	} exitBy;

	union {
		int exitStatus;
		int terminatedBySignal;
		int stopSignal;
	};
} processState_t;

#ifdef __cplusplus
extern "C"
{
#endif

	THROWS err_t getProcessName(const pid_t pid, char *buf, int bufSize);
	THROWS err_t getThreadName(const pid_t pid, const pid_t tid, char *buf, int bufSize);

	THROWS err_t killProcess(const pid_t pid);
	THROWS err_t safeWaitPid(const pid_t pid, processState_t *status, int options);
	THROWS err_t safeSleep(const struct timespec *req);

	THROWS err_t setUserPolicy(const uid_t uid, const gid_t gid);

#ifdef __cplusplus
}
#endif
