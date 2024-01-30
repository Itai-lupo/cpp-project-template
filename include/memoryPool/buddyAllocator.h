#pragma once
#include "types/err_t.h"
#include "types/fd_t.h"
#include <cstddef>
#include <unistd.h>

#include "memoryPool/lowLevelPoolInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif
	THROWS err_t initBuddyAllocator(lowLevelPoolInfo pool, size_t maxSize, size_t minAllocation);
	THROWS err_t closeBuddyAllocator();

	THROWS err_t buddyAlloc(void **ptr, size_t size);
	THROWS err_t buddyFree(void **ptr);
#ifdef __cplusplus
}
#endif
