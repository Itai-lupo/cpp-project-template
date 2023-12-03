#include "core.hpp"
#include "log.h"


int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	initLogger();
	LOG_INFO("hello world");
	closeLogger();

	return 0;
}
