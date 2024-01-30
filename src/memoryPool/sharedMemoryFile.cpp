#include "memoryPool/sharedMemoryFile.h"
#include "defaultTrace.h"

#include "err.h"
#include "files.h"

#include <linux/memfd.h>
#include <sys/mman.h>

size_t maxSize = 0;
size_t *currentSize = nullptr;
void *startAddr = nullptr;
fd_t memfd = INVALID_FD;

THROWS err_t initSharedMemoryFile(size_t _maxSize)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(_maxSize > 0);
	QUITE_CHECK(startAddr == nullptr);
	QUITE_CHECK(IS_INVALID_FD(memfd));

	memfd.fd = memfd_create("shared memory pool", MFD_HUGETLB | MFD_HUGE_2MB);
	QUITE_CHECK(IS_VALID_FD(memfd));

	currentSize = new size_t(0);


	maxSize = _maxSize;
	startAddr =
		mmap(NULL, maxSize, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE | MAP_HUGETLB | MAP_NORESERVE, memfd.fd, 0);
	QUITE_CHECK(startAddr != MAP_FAILED);

cleanup:
	return err;
}

err_t closeSharedMemoryFile()
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(maxSize > 0);
	QUITE_CHECK(startAddr != nullptr);
	QUITE_CHECK(IS_VALID_FD(memfd));

	QUITE_CHECK(munmap(startAddr, maxSize) == 0);
	startAddr = nullptr;

	QUITE_RETHROW(safeClose(&memfd));
cleanup:
	return err;
}

THROWS err_t setSharedMemoryFileSize(size_t size)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(currentSize != nullptr);
	QUITE_CHECK(IS_VALID_FD(memfd));
	CHECK_TRACE((size % (1 << 21)) == 0, "size is not huge pages allinged %lu %lu", (size % (1 << 21)), size);

	QUITE_CHECK(ftruncate(memfd.fd, size) == 0);
	*currentSize = size;
cleanup:
	return err;
}

THROWS err_t getSharedMemoryFileSize(size_t *size)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(IS_VALID_FD(memfd));
	QUITE_CHECK(currentSize != nullptr);

	QUITE_CHECK(size != nullptr);
	*size = *currentSize;
cleanup:
	return err;
}

THROWS err_t getSharedMemoryFileFd(fd_t *fd)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(startAddr != nullptr);
	QUITE_CHECK(IS_VALID_FD(memfd));

	QUITE_CHECK(fd != nullptr);

	*fd = memfd;
cleanup:
	return err;
}

THROWS err_t getSharedMemoryFileStartAddr(void **ptr)
{
	err_t err = NO_ERRORCODE;

	QUITE_CHECK(startAddr != nullptr);
	QUITE_CHECK(IS_VALID_FD(memfd));

	QUITE_CHECK(ptr != nullptr);

	*ptr = startAddr;
cleanup:
	return err;
}
