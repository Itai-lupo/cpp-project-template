#pragma once

#include "types/err_t.h"

#include <bits/types/struct_timespec.h>
#include <sched.h>
#include <unistd.h>

THROWS err_t getProcessName(const pid_t pid, char *buf, int bufSize);
THROWS err_t getThreadName(const pid_t pid, const pid_t tid, char *buf, int bufSize);

THROWS err_t killProcess(const pid_t pid);
THROWS err_t safeWaitPid(const pid_t pid);
THROWS err_t safeNonblockingWaitPid(const pid_t pid);
THROWS err_t safeSleep(const struct timespec *req);
THROWS err_t setUserPolicy(const uid_t uid, const gid_t gid);
