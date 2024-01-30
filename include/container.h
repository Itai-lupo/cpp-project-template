/**
 * @file container.h
 * @author itai lupo
 * @brief run code inside a container process.
 * @version 0.1
 * @date 2023-12-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // !_GNU_SOURCE

#include "types/err_t.h"

typedef err_t (*containerCallback)(void *);

#ifdef __cplusplus
extern "C"
{
#endif

	THROWS err_t runInContainer(containerCallback callback, void *data, pid_t *childPid);

#ifdef __cplusplus
}
#endif
