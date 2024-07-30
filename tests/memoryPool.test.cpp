#include "log.h"

#include "err.h"
#include "types/memoryAllocator.h"
#include "processes.h"
#include "allocators/sharedMemoryPool.h"

#include <gtest/gtest.h>
#include <unistd.h>

TEST(sharedMemoryPool, allocAndDealloc)
{
	err_t err = NO_ERRORCODE;
	int *data = nullptr;

	RETHROW(sharedAlloc((void **)&data, 1, sizeof(int), 1));
	ASSERT_NE(data, nullptr);

	*data = 1;
	ASSERT_EQ(*data, 1);

	RETHROW(sharedDealloc((void **)&data));
	ASSERT_EQ(data, nullptr);

cleanup:
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, allocAndDeallocAndFork)
{
	err_t err = NO_ERRORCODE;
	int *data = NULL;
	pid_t pid = 0;
	processState_t status;

	RETHROW(sharedAlloc((void **)&data, 1, sizeof(int), 1));
	ASSERT_NE(data, nullptr);

	*data = 1;
	ASSERT_EQ(*data, 1);

	pid = fork();
	CHECK(pid >= 0);

	if (pid == 0)
	{
		*data = 2;
		ASSERT_EQ(*data, 2);
		exit(0);
	}

	RETHROW(safeWaitPid(pid, &status, 0));
	ASSERT_TRUE(status.exitBy.normal) << "exited by sig " << status.terminatedBySignal;
	ASSERT_EQ(status.exitStatus, 0) << "exited with value" << status.exitStatus;

	ASSERT_EQ(*data, 2);

	RETHROW(sharedDealloc((void **)&data));
	ASSERT_EQ(data, nullptr);

cleanup:
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, forkAndThenMallocAndPlaceInMallocedPtr)
{
	err_t err = NO_ERRORCODE;
	int **dataPtr = NULL;
	pid_t pid = 0;
	processState_t status;

	RETHROW(sharedAlloc((void **)&dataPtr, 1, sizeof(int *), 1));
	pid = fork();
	CHECK(pid >= 0);

	if (pid == 0)
	{
		RETHROW(sharedAlloc((void **)dataPtr, 1, sizeof(int), 1));
		ASSERT_NE(dataPtr, nullptr);
		**dataPtr = 2;
		ASSERT_EQ(**dataPtr, 2);
		exit(0);
	}

	RETHROW(safeWaitPid(pid, &status, 0));
	ASSERT_EQ((**dataPtr), 2);
	RETHROW(sharedDealloc((void **)dataPtr));
	RETHROW(sharedDealloc((void **)&dataPtr));
	ASSERT_EQ(dataPtr, nullptr);

cleanup:

	ASSERT_TRUE(status.exitBy.normal) << "exited by sig " << status.terminatedBySignal;
	ASSERT_EQ(status.exitStatus, 0) << "exited with value " << status.exitStatus;

	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, useALotOfMemory)
{
	err_t err = NO_ERRORCODE;
	char *data = NULL;
	constexpr size_t dataSetSize = 4000000;

	RETHROW(sharedAlloc((void **)&data, dataSetSize, sizeof(char), 0));
	ASSERT_NE(data, nullptr);

	data[dataSetSize - 1] = 0;
	// memset(data, 0, dataSetSize);

	RETHROW(sharedDealloc((void **)&data));
	ASSERT_EQ(data, nullptr);

cleanup:
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, useAllTheMemory)
{
	err_t err = NO_ERRORCODE;
	int *data = NULL;

	while (true)
	{

		QUITE_RETHROW(sharedAlloc((void **)&data, 100000, sizeof(int), ALLOCATOR_CLEAR_MEMORY));
		ASSERT_NE(data, nullptr);
		for (int i = 0; i < 100000; i++) {
			data[i] = 0;
		
		}
		data = nullptr;
	}
cleanup:

	err = NO_ERRORCODE;
	ASSERT_TRUE(!IS_ERROR(err));
}
