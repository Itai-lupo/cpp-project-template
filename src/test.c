#include "test.h"
#include "log.h"

void logFromC()
{
	err_t err = NO_ERRORCODE;
	LOG_INFO("hello from %s", "c");
	CHECK(false);
cleanup:
    REWARN(err);
}
