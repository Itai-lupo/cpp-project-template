#include "log.h"

#include "allocators/sharedMemoryPool.h"
#include "err.h"
#include "processes.h"
#include "types/memoryAllocator.h"

#include <gtest/gtest.h>
#include <unistd.h>

TEST(sharedMemoryPool, allocAndDealloc)
{
	err_t err = NO_ERRORCODE;
	int *data = nullptr;

	RETHROW(sharedAlloc((void **)&data, 1, sizeof(int), 1, NULL));
	ASSERT_NE(data, nullptr);

	*data = 1;
	ASSERT_EQ(*data, 1);

	RETHROW(sharedDealloc((void **)&data, NULL));
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

	RETHROW(sharedAlloc((void **)&data, 1, sizeof(int), 1, NULL));
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

	RETHROW(sharedDealloc((void **)&data, NULL));
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

	RETHROW(sharedAlloc((void **)&dataPtr, 1, sizeof(int *), 1, NULL));
	pid = fork();
	CHECK(pid >= 0);

	if (pid == 0)
	{
		RETHROW(sharedAlloc((void **)dataPtr, 1, sizeof(int), 1, NULL));
		ASSERT_NE(dataPtr, nullptr);
		**dataPtr = 2;
		ASSERT_EQ(**dataPtr, 2);
		exit(0);
	}

	RETHROW(safeWaitPid(pid, &status, 0));
	ASSERT_EQ((**dataPtr), 2);
	RETHROW(sharedDealloc((void **)dataPtr, NULL));
	RETHROW(sharedDealloc((void **)&dataPtr, NULL));
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

	RETHROW(sharedAlloc((void **)&data, dataSetSize, sizeof(char), 0, NULL));
	ASSERT_NE(data, nullptr);

	data[dataSetSize - 1] = 0;
	bzero(data, dataSetSize);

	RETHROW(sharedDealloc((void **)&data, NULL));
	ASSERT_EQ(data, nullptr);

cleanup:
	ASSERT_TRUE(!IS_ERROR(err));
}

TEST(sharedMemoryPool, allocateAndFreeAlot)
{
	err_t err = NO_ERRORCODE;
	int *data[40000] = {NULL};

	for(int i = 0; i < 40000; i++)
	{
		RETHROW(sharedAlloc((void **)&data[i], 510, sizeof(int), 0, NULL));
		
    ASSERT_NE(data[i], nullptr);

    memset(data[i], 0xFFFFFFFF, 510 * sizeof(int));
	}

	for(int i = 0; i < 40000; i++)
  {
  	RETHROW(sharedDealloc((void **)&data[i], NULL));

    ASSERT_EQ(data[i], nullptr);
  }

cleanup:
  REWARN(err);
	ASSERT_TRUE(!IS_ERROR(err));
}

//no need to run that test every time as it is long and uses around 8-16GiB of ram. 
TEST(sharedMemoryPool, DISABLED_useAllTheMemory)
{
	err_t err = NO_ERRORCODE;
	int *data = NULL;
  size_t j = 0;
	while (true)
	{
    j++;
		QUITE_RETHROW(sharedAlloc((void **)&data, 510, sizeof(int), 0, NULL));
		ASSERT_NE(data, nullptr);

		data = nullptr;
	}
cleanup:
  REWARN(err);
  LOG_INFO("{}", j);
	ASSERT_TRUE(err.errorCode == ENOMEM);
}

static void  __attribute__((noreturn)) allocateAndFreeALotOfMemory(){
  err_t err = NO_ERRORCODE;
  pid_t pid = 0; 
  pid_t *data[4000] = {NULL};
  
  pid = getpid();

  for(int i = 0; i < 4000; i++)
	{
		RETHROW(sharedAlloc((void **)&data[i], 510, sizeof(pid_t), 0, NULL));
		
    CHECK_NOTRACE_ERRORCODE(data[i] != nullptr, ENOMEM);

    memset(data[i], pid, 510 * sizeof(pid_t));
	}

	for(int i = 0; i < 4000; i++)
  {
    for(int j = 0; j < 510; j++)
    {
      CHECK_NOTRACE_ERRORCODE(data[i][j] == pid, EINVAL);
    } 

  	RETHROW(sharedDealloc((void **)&data[i], NULL));

    CHECK_NOTRACE_ERRORCODE(data[i] == nullptr, EFAULT);
  }

cleanup:
  REWARN(err);
  exit(err.errorCode);
}

TEST(sharedMemoryPool, allocateAndFreeAlotButForkAlot)
{
	err_t err = NO_ERRORCODE;
  pid_t pids[100] = {0};
	processState_t childsExitStatus[100];
  int childCount = 0;

  for(childCount = 0; childCount < 100; childCount++)
  {
    pids[childCount] = fork();
    QUITE_CHECK(pids[childCount] != -1);
    if(pids[childCount] == 0)
    {
      allocateAndFreeALotOfMemory();
    }
  }
	
cleanup:
  for(int i = 0; i < childCount; i++)
  {
	  REWARN(safeWaitPid(pids[i], &childsExitStatus[i], 0));
  }

  for(int i = 0; i < childCount; i++){
	  ASSERT_TRUE(childsExitStatus[i].exitBy.normal) << "exited by sig " << childsExitStatus[i].terminatedBySignal;
	  ASSERT_EQ(childsExitStatus[i].exitStatus, 0) << "exited with value " << childsExitStatus[i].exitStatus;
  }

  REWARN(err);
	ASSERT_TRUE(!IS_ERROR(err));
}


