
#include "log.h"

#include "err.h"

#include <gtest/gtest.h>

#include "hotReload.h"

const constexpr char *TEST_SYSTEM_NAME = "test system";

TEST(hotReloadSystem, loadAndUloadSystem)
{
	err_t err = NO_ERRORCODE;
	systemId_t id = INVALID_SYSTEM_ID;

	RETHROW(loadSystem(TEST_SYSTEM_NAME, &id));

cleanup:
	REWARN(unloadSystem(id));

	ASSERT_TRUE(!IS_ERROR(err));
}
