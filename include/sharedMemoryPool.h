#pragma once
#include "types/err_t.h"
#include <cstddef>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif
	THROWS err_t initSharedMemory();
	err_t closeSharedMemory();

	THROWS err_t sharedAlloc(void **const data, const size_t size);
	THROWS err_t sharedDealloc(void **const data);

#ifdef __cplusplus
}
#endif
