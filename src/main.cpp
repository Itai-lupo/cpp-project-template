#include <iostream>
#include <sys/cdefs.h>

#include "core.hpp"

#include "a.hpp"

int main ([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    std::cout << "hello world" << std::endl;
    spdlog::info("test");

    
    SPDLOG_TRACE("Some trace message with param {}", 42);
    SPDLOG_DEBUG("Some debug message");
    spdlog::info("d");
    return 0;
}
