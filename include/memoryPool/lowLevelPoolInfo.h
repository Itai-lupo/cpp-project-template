#pragma once
#include "types/err_t.h"

typedef err_t (*getPoolSizeCallback)(size_t *size);
typedef err_t (*setPoolSizeCallback)(size_t size);

typedef struct
{
	void *startAddr;
	getPoolSizeCallback getSize;
	setPoolSizeCallback setSize;
} lowLevelPoolInfo;
