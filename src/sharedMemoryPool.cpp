#include "sharedMemoryPool.h"
#include "defaultTrace.h"
#include "err.h"
#include "files.h"
#include "memoryPool/buddyAllocator.h"
#include "memoryPool/lowLevelPoolInfo.h"
#include "memoryPool/sharedMemoryFile.h"
#include "memoryPool/types/memoryAllocator.h"

#include <cstdint>
#include <math.h>
#include <string.h>
#include <sys/mman.h>

// this 16GiB(2^34B) most system won't even by able to allocate that much ever so there will be errors out of memry most
// likly before we reach that
static const size_t MAX_RANGE = pow(2, 34);

// min block size is 16MiB to save on mangment size, this is the allocater that give blocks to the local pools there fore it dosn't need less then that
static const size_t MIN_BUDDY_BLOCK_SIZE = pow(2, 21);

static const off_t poolSize = sysconf(_SC_PAGESIZE) * 16;


THROWS err_t tempAlloc(void **const data, const size_t count, const size_t size, allocatorFlags flags)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(data != NULL);
	QUITE_CHECK(*data == NULL);
	QUITE_CHECK(size > 0);

    *data = malloc(size * count);

	if (flags || ALLOCATOR_CLEAR_MEMORY == 1)
	{
		memset(*data, 0, count * size);
	}

cleanup:
	return err;
}



static memoryAllocator allocator = {&sharedAlloc, &sharedRealloc, &sharedDealloc};

THROWS err_t initSharedMemory()
{
	err_t err = NO_ERRORCODE;
	lowLevelPoolInfo poolInterface = {nullptr, getSharedMemoryFileSize, setSharedMemoryFileSize};

	QUITE_RETHROW(initSharedMemoryFile(MAX_RANGE));
	QUITE_RETHROW(getSharedMemoryFileStartAddr(&poolInterface.startAddr));
    QUITE_RETHROW(setSharedMemoryFileSize(MIN_BUDDY_BLOCK_SIZE));

	QUITE_RETHROW(initBuddyAllocator(poolInterface, &allocator, MAX_RANGE, MIN_BUDDY_BLOCK_SIZE));

	// QUITE_RETHROW(copyIntoNewAlloctor(&allocator));

cleanup:
	return err;
}

err_t closeSharedMemory()
{
	err_t err = NO_ERRORCODE;

	QUITE_RETHROW(closeBuddyAllocator());
	

    QUITE_RETHROW(closeSharedMemoryFile());

cleanup:
	return err;
}

THROWS err_t sharedAlloc(void **const data, const size_t count, const size_t size, allocatorFlags flags)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(data != NULL);
	QUITE_CHECK(*data == NULL);
	QUITE_CHECK(size > 0);

	QUITE_RETHROW(buddyAlloc(data, size * count));

	if (flags || ALLOCATOR_CLEAR_MEMORY == 1)
	{
		memset(*data, 0, count * size);
	}

cleanup:
	return err;
}


THROWS err_t sharedRealloc(void **const data, const size_t count, const size_t size, allocatorFlags flags)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(data != NULL);
	QUITE_CHECK(*data != NULL);
	QUITE_CHECK(size > 0);

	QUITE_RETHROW(buddyFree(data));
	QUITE_RETHROW(buddyAlloc(data, size * count));

	if (flags || ALLOCATOR_CLEAR_MEMORY == 1)
	{
		memset(*data, 0, count * size);
	}

cleanup:
	return err;
}

THROWS err_t sharedDealloc(void **const data)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(data != NULL);
	QUITE_CHECK(*data != NULL);

	QUITE_RETHROW(buddyFree(data));

cleanup:
	if (data != NULL)
	{
		*data = NULL;
	}

	return err;
}
