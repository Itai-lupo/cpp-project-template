
#include "log.h"

#include "err.h"
#include "allocators/sharedMemoryPool.h"
#include "processes.h"

#include <types/safeQueue.h>
#include <gtest/gtest.h>
#include <unistd.h>


TEST(safeQueue, initQueue)
{
    err_t err = NO_ERRORCODE;
    safeQueue *Q = NULL;

    RETHROW(initSafeQueue(8, getSharedAllocator(), 1, &Q)); 
    RETHROW(freeSafeQueue(&Q));

cleanup:
	ASSERT_FALSE(IS_ERROR(err));
}


TEST(safeQueue, pushAndPop)
{
    err_t err = NO_ERRORCODE;
    safeQueue *Q = NULL;
    int *buf = NULL;
    int res = 0;

    RETHROW(initSafeQueue(sizeof(int), getSharedAllocator(), 1, &Q)); 

    RETHROW(safeQueueGetEmptyNode(Q, (void**)&buf));
    *buf = 1;
    RETHROW(safeQueuePush(Q, buf));
    RETHROW(safeQueuePop(Q, &res, sizeof(int)));
    
    ASSERT_EQ(res, 1);

    RETHROW(freeSafeQueue(&Q));

cleanup:
	ASSERT_FALSE(IS_ERROR(err));


}


TEST(safeQueue, manyPushsAndPops)
{

 err_t err = NO_ERRORCODE;
    safeQueue *Q = NULL;
    int *buf = NULL;
    int res = 0;

    RETHROW(initSafeQueue(sizeof(int), getSharedAllocator(), 1, &Q)); 

    for (int i = 0; i < 100; i++) 
    {
        buf = NULL;
        RETHROW(safeQueueGetEmptyNode(Q, (void**)&buf));
        *buf = i;
        RETHROW(safeQueuePush(Q, buf));
    }
    
    for(int i = 0; i < 100; i++)
    {
        RETHROW(safeQueuePop(Q, &res, sizeof(int)));
        
        ASSERT_EQ(res, i);
    }

    RETHROW(freeSafeQueue(&Q));

cleanup:
	ASSERT_FALSE(IS_ERROR(err));

}

struct PidCountTuple
{
    pid_t pid;
    int count;
};

static void __attribute__((noreturn)) pushAndPopAlot(safeQueue *Q)
{
    err_t err = NO_ERRORCODE;
    PidCountTuple *buf = NULL;
    
    for (int i = 0; i < 10000; i++) 
    {
        buf = NULL;
        RETHROW(safeQueueGetEmptyNode(Q, (void**)&buf));
        // LOG_INFO("{}", i);
        buf->count = i;
        buf->pid = getpid();
        RETHROW(safeQueuePush(Q, buf));
    }
    

cleanup:
	exit(err.errorCode);
}

TEST(safeQueue, manyProcessPushsAndPops)
{
err_t err = NO_ERRORCODE;
	pid_t pids[2000] = {0};
	processState_t childsExitStatus[2000];
    safeQueue *Q = NULL;
    PidCountTuple res = {0, 0};
    std::map<pid_t, int> processCounters;
    bool isEmpty;
    int counter = 0;
	int childCount = 0;
    RETHROW(initSafeQueue(sizeof(PidCountTuple), getSharedAllocator(), 1, &Q)); 

	for (childCount = 0; childCount < 20; childCount++)
	{
		pids[childCount] = fork();
		QUITE_CHECK(pids[childCount] != -1);
		if (pids[childCount] == 0)
		{
			pushAndPopAlot(Q);
		}
        processCounters[res.pid] = 0;
	}

    for(int i = 0; i < 10000 * childCount; i++)
    {
        isEmpty = true;
        counter = 0;
        while(isEmpty && counter < 10000){
            counter++;
            if(counter == 10000){
                goto cleanup;
            }
            RETHROW(safeQueueIsEmpty(Q, &isEmpty));
        }

        RETHROW(safeQueuePop(Q, &res, sizeof(PidCountTuple)));
        
        ASSERT_EQ(res.count, processCounters[res.pid]) << "for pid: " << res.pid << " at pop: " << i;
        processCounters[res.pid]++;
    }

cleanup:
	for (int i = 0; i < childCount; i++)
	{
		REWARN(safeWaitPid(pids[i], &childsExitStatus[i], 0));
	}

	for (int i = 0; i < childCount; i++)
	{
		ASSERT_TRUE(childsExitStatus[i].exitBy.normal) << "exited by sig " << childsExitStatus[i].terminatedBySignal;
		ASSERT_EQ(childsExitStatus[i].exitStatus, 0) << "exited with value " << childsExitStatus[i].exitStatus;
	}

	REWARN(err);
    REWARN(freeSafeQueue(&Q));
	ASSERT_TRUE(!IS_ERROR(err));

}
