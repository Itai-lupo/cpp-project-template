#pragma once

#include "core.hpp"
#include <cstdint>

#define INVALID_SYSTEM_ID {.id = UINT64_MAX}
#define IS_VALID_SYSTEMID(systemId) ((systemId).id != UINT64_MAX)

typedef struct
{
	uint64_t id;

} systemId_t;

err_t loadSystem(const char *systemName, systemId_t *res);

err_t unloadSystem(systemId_t systemId);
