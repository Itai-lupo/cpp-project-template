#include "log.h"
#include "test.h"

void logFromC()
{
	err_t err = NO_ERRORCODE;
	LOG_INFO("hello from %s", "c");
	CHECK(false);
cleanup:
}
