#include "hotReload.h"
#include "log.h"

#include "err.h"

#include <cstdlib>

err_t loadSystem(const char *systemName, systemId_t *res)
{
	err_t err = NO_ERRORCODE;
	CHECK(res != nullptr);
	CHECK(systemName != nullptr);

	CHECK(!IS_VALID_SYSTEMID(*res));

    res->id = 1;

    // in order to load a system we need
    // 1. find the so for the system
    // 2. allocate the system handle(a set of pointer to the state the system should load and every fd it might need)
    // 3. fork and load the system so.
    // 4. call the system init callback
    // 4. set watcher on the system so file
    //
    // both the allocate handle and init function should be exported symboles on the system so and be called from it.

cleanup:
	return err;
}

err_t unloadSystem(systemId_t systemId)
{
	err_t err = NO_ERRORCODE;

	CHECK(IS_VALID_SYSTEMID(systemId));

cleanup:
	return err;
}
