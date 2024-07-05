#pragma once
#include "types/err_t.h"
#include "types/fd_t.h"
#include <cstddef>
#include <unistd.h>

#include "memoryPool/lowLevelPoolInfo.h"
#include "memoryPool/types/memoryAllocator.h"

#ifdef __cplusplus
extern "C"
{
#endif
	THROWS err_t initBuddyAllocator(lowLevelPoolInfo pool, const memoryAllocator *alloctor, const size_t maxSize,
									const size_t minAllocation);
	THROWS err_t closeBuddyAllocator();

	THROWS err_t copyIntoNewAlloctor(const memoryAllocator *alloctor);

	THROWS err_t buddyAlloc(void **const ptr, size_t size);
	THROWS err_t buddyFree(void **const ptr);
#ifdef __cplusplus
}
#endif
