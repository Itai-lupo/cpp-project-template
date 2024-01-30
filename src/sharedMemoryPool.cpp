#include "sharedMemoryPool.h"
#include "defaultTrace.h"
#include "err.h"
#include "files.h"
#include "memoryPool/buddyAllocator.h"
#include "memoryPool/lowLevelPoolInfo.h"
#include "memoryPool/sharedMemoryFile.h"

#include <cstdint>
#include <math.h>
#include <sys/mman.h>

// this 16GiB(2^34B) most system won't even by able to allocate that much ever so there will be errors out of memry most
// likly before we reach that
static const size_t MAX_RANGE = pow(2, 34);
static const size_t MIN_BUDDY_BLOCK_SIZE = pow(2, 21);

static const off_t poolSize = sysconf(_SC_PAGESIZE) * 16;

THROWS err_t initSharedMemory()
{
	err_t err = NO_ERRORCODE;
	lowLevelPoolInfo poolInterface = {nullptr, getSharedMemoryFileSize, setSharedMemoryFileSize};

	QUITE_RETHROW(initSharedMemoryFile(MAX_RANGE));
	QUITE_RETHROW(getSharedMemoryFileStartAddr(&poolInterface.startAddr));

	QUITE_RETHROW(initBuddyAllocator(poolInterface, MAX_RANGE, MIN_BUDDY_BLOCK_SIZE));

	QUITE_RETHROW(setSharedMemoryFileSize(MIN_BUDDY_BLOCK_SIZE * 10));
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

THROWS err_t sharedAlloc(void **const data, const size_t size)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(data != NULL);
	QUITE_CHECK(*data == NULL);
	QUITE_CHECK(size > 0);

	QUITE_RETHROW(buddyAlloc(data, size));

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
