#include "log.h"

#include "err.h"
#include "processes.h"
#include "sharedMemoryPool.h"

#include <climits>
#include <cstdint>
#include <gtest/gtest.h>
#include <unistd.h>
TEST(sharedMemoryPool, initAndClose)
{
	err_t err = NO_ERRORCODE;
	RETHROW(initSharedMemory());

	RETHROW(closeSharedMemory());

cleanup:
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, allocAndDealloc)
{
	err_t err = NO_ERRORCODE;
	int *data = nullptr;

	RETHROW(initSharedMemory());

	RETHROW(sharedAlloc((void **)&data, sizeof(int)));
	ASSERT_NE(data, nullptr);

	*data = 1;
	ASSERT_EQ(*data, 1);

	RETHROW(sharedDealloc((void **)&data));
	ASSERT_EQ(data, nullptr);

cleanup:
	REWARN(closeSharedMemory());
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, allocAndDeallocAndFork)
{
	err_t err = NO_ERRORCODE;
	int *data = NULL;
	pid_t pid = 0;
	processState_t status;

	RETHROW(initSharedMemory());

	RETHROW(sharedAlloc((void **)&data, sizeof(int)));
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
	REWARN(closeSharedMemory());
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, forkAndThenMallocAndPlaceInMallocedPtr)
{
	err_t err = NO_ERRORCODE;
	int **dataPtr = NULL;
	pid_t pid = 0;
	processState_t status;

	RETHROW(initSharedMemory());

	RETHROW(sharedAlloc((void **)&dataPtr, sizeof(int *)));
	pid = fork();
	CHECK(pid >= 0);

	if (pid == 0)
	{
		RETHROW(sharedAlloc((void **)dataPtr, sizeof(int)));
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
	REWARN(closeSharedMemory());

	ASSERT_TRUE(status.exitBy.normal) << "exited by sig " << status.terminatedBySignal;
	ASSERT_EQ(status.exitStatus, 0) << "exited with value " << status.exitStatus;

	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, useALotOfMemory)
{
	err_t err = NO_ERRORCODE;
	char *data = NULL;
	constexpr size_t dataSetSize = 4096 * 16 + 1; // *4000000;

	RETHROW(initSharedMemory());
	RETHROW(sharedAlloc((void **)&data, dataSetSize));
	ASSERT_NE(data, nullptr);

	memset(data, 0, dataSetSize);

	RETHROW(sharedDealloc((void **)&data));
	ASSERT_EQ(data, nullptr);

cleanup:
	REWARN(closeSharedMemory());
	ASSERT_TRUE(!IS_ERROR(err));
}
