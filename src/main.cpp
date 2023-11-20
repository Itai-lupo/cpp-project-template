#include "core.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  spdlog::info("hello world");
  spdlog::error("error");

  spdlog::set_pattern("[%H:%M:S %z]");
  SPDLOG_DEBUG("debug message to default logger");
  return 0;
}
