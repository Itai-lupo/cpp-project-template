#include "memoryPool/buddyAllocator.h"

// #include "defaultTrace.h"
#include "log.h"

#include "err.h"
#include "types/fd_t.h"

#include <math.h>
#include <sys/param.h>

#define IS_POWER_OF_2(x) ((x & (x - 1)) == 0)

lowLevelPoolInfo memoryPoolInterface = {nullptr, nullptr, nullptr};

static size_t freeListsCount = 0;
static size_t maxSize = 0;
static size_t minAllocation = 0;

typedef struct
{
	void **list;
	size_t freeItems;
	size_t listMaxSize;
} freeList_t;

static freeList_t *freeLists;

THROWS err_t initBuddyAllocator(lowLevelPoolInfo pool, size_t _maxSize, size_t _minAllocation)
{
	err_t err = NO_ERRORCODE;

	maxSize = _maxSize;
	minAllocation = _minAllocation;

	QUITE_CHECK(memoryPoolInterface.startAddr == nullptr);
	QUITE_CHECK(pool.startAddr != nullptr);

	QUITE_CHECK(IS_POWER_OF_2(maxSize))
	QUITE_CHECK(IS_POWER_OF_2(minAllocation))

	memoryPoolInterface = pool;

	freeListsCount = log2(maxSize / minAllocation);
	// LOG_INFO("max allocation = {}, min allocation = {}, freeListsCount = {}", maxSize, minAllocation,
	// freeListsCount);

	freeLists = (freeList_t *)calloc(freeListsCount + 1, sizeof(freeList_t));
	QUITE_CHECK(freeLists != NULL);

	for (size_t i = 0; i <= freeListsCount; i++)
	{
		freeLists[i].listMaxSize = pow(2, freeListsCount - i);
		freeLists[i].list = (void **)calloc(freeLists[i].listMaxSize, sizeof(void *));
		freeLists[i].freeItems = 0;

		QUITE_CHECK(freeLists[i].list != NULL);

		// LOG_INFO("list {} has max items of {} with {} item size", i, freeLists[i].listMaxSize,
		// pow(2, log2(maxSize) - (freeListsCount - i)));
	}

	freeLists[freeListsCount].freeItems = 1;
	freeLists[freeListsCount].list[0] = memoryPoolInterface.startAddr;

cleanup:
	return err;
}

THROWS err_t closeBuddyAllocator()
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(memoryPoolInterface.startAddr != nullptr);

	memoryPoolInterface = {nullptr, nullptr, nullptr};
	LOG_INFO("close");
cleanup:
	return err;
}

THROWS err_t buddyAlloc(void **ptr, size_t size)
{
	err_t err = NO_ERRORCODE;
	size_t poolSize = 0;
	size_t wantedPowerOf2 = 0;
	int i = 0;
	uint8_t *order = nullptr;

	// mangment data
	size++;

	QUITE_CHECK(memoryPoolInterface.startAddr != nullptr);

	QUITE_CHECK(ptr != NULL);
	QUITE_CHECK(size > 0);

	QUITE_RETHROW(memoryPoolInterface.getSize(&poolSize));

	wantedPowerOf2 = ceil(log2((double)MAX(size, minAllocation)) - log2((double)minAllocation));

	QUITE_CHECK(freeListsCount >= wantedPowerOf2);

	if (freeLists[wantedPowerOf2].freeItems > 0)
	{
		*ptr = freeLists[wantedPowerOf2].list[freeLists[wantedPowerOf2].freeItems - 1];
		freeLists[wantedPowerOf2].list[freeLists[wantedPowerOf2].freeItems - 1] = nullptr;
		freeLists[wantedPowerOf2].freeItems--;
	}
	else
	{
		i = wantedPowerOf2 + 1;
		while (freeLists[i].freeItems == 0 && (size_t)i <= freeListsCount)
		{
			i++;
		}

		QUITE_CHECK((size_t)i <= freeListsCount);

		*ptr = freeLists[i].list[freeLists[i].freeItems - 1];

		freeLists[i].list[freeLists[i].freeItems - 1] = nullptr;
		freeLists[i].freeItems--;

		i--;

		for (; i >= (int)wantedPowerOf2; i--)
		{
			QUITE_CHECK(freeLists[i].freeItems <= freeLists[i].listMaxSize);
			freeLists[i].list[freeLists[i].freeItems] = (void *)((char *)(*ptr) + (size_t)(minAllocation * pow(2, i)));
			freeLists[i].freeItems++;
		}
	}

	order = (uint8_t *)*ptr;
	*order = wantedPowerOf2;
	*ptr = (char *)*ptr + 1;

	if (((char *)*ptr - (char *)memoryPoolInterface.startAddr) + pow(2, wantedPowerOf2) > poolSize)
	{
		LOG_INFO("set new size to {}",
				 (size_t)((char *)*ptr - (char *)memoryPoolInterface.startAddr) + pow(2, wantedPowerOf2));
		QUITE_RETHROW(memoryPoolInterface.setSize(((char *)*ptr - (char *)memoryPoolInterface.startAddr) +
												  pow(2, wantedPowerOf2)))
	}

cleanup:
	return err;
}

THROWS err_t buddyFree(void **ptr)
{
	err_t err = NO_ERRORCODE;
	uint8_t order = 0;
	size_t size = 0;
	void *buddyAddr = nullptr;
	bool foundBuddy = true;

	QUITE_CHECK(ptr != NULL);
	QUITE_CHECK(memoryPoolInterface.startAddr != nullptr);
	QUITE_CHECK(*ptr >= memoryPoolInterface.startAddr && *ptr <= (char *)memoryPoolInterface.startAddr + maxSize);

	order = *((uint8_t *)*ptr - 1);
	*ptr = (char *)*ptr - 1;

	for (; order <= freeListsCount && foundBuddy; order++)
	{
		foundBuddy = false;

		size = minAllocation * pow(2, order);

		buddyAddr = (void *)((char *)*ptr +
							 (1 - ((((char *)*ptr - (char *)memoryPoolInterface.startAddr) / size) % 2) * 2) * size);

		LOG_INFO("offset of {} size {} from start in order {} buddy addr is {}", *ptr, size, order, buddyAddr);

		for (size_t i = 0; i < freeLists[order].freeItems; i++)
		{
			if (freeLists[order].list[i] == buddyAddr)
			{
				foundBuddy = true;
				LOG_INFO("found buddy at {} with order {}", i, order);
				freeLists[order].list[i] = freeLists[order].list[freeLists[order].freeItems - 1];
				freeLists[order].freeItems--;

				freeLists[order + 1].list[freeLists[order + 1].freeItems] = MIN(buddyAddr, *ptr);
				freeLists[order + 1].freeItems++;
			}
		}

		*ptr = MIN(buddyAddr, *ptr);
	}

	*ptr = nullptr;
cleanup:
	return err;
}
