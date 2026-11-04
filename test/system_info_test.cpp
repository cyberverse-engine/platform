#include <catch2/catch_test_macros.hpp>

#include "cyberverse/platform.hpp"

using cyberverse::platform::backend_type;
using cyberverse::platform::make_platform;
using cyberverse::platform::memory_info;
using cyberverse::platform::process_info;
using cyberverse::platform::system_info;

TEST_CASE("platform reports basic system information", "[platform][system_info]") {
  auto host = make_platform(backend_type::headless);
  REQUIRE(host != nullptr);

  const system_info info = host->query_system_info();
  CHECK_FALSE(info.os_name.empty());
  CHECK_FALSE(info.architecture.empty());
  CHECK(info.logical_core_count > 0);
  CHECK(info.total_memory_bytes > 0);
}

TEST_CASE("platform reports available memory", "[platform][system_info]") {
  auto host = make_platform(backend_type::headless);
  const system_info system = host->query_system_info();
  const memory_info memory = host->query_memory_info();

  CHECK(memory.available_memory_bytes > 0);
  if (system.total_memory_bytes > 0) {
    CHECK(memory.available_memory_bytes <= system.total_memory_bytes);
  }
}

TEST_CASE("platform reports process memory", "[platform][system_info]") {
  auto host = make_platform(backend_type::headless);
  const process_info process = host->query_process_info();

  CHECK(process.resident_bytes > 0);
  CHECK(process.virtual_bytes > 0);
}

TEST_CASE("platform system information can be queried repeatedly", "[platform][system_info]") {
  auto host = make_platform(backend_type::headless);

  const system_info first = host->query_system_info();
  const system_info second = host->query_system_info();

  CHECK(first.os_name == second.os_name);
  CHECK(first.logical_core_count == second.logical_core_count);
  CHECK(first.total_memory_bytes == second.total_memory_bytes);
}

TEST_CASE("platform reports cumulative CPU times when available", "[platform][system_info]") {
  auto host = make_platform(backend_type::headless);
  const auto first = host->query_cpu_times();
  if (!first.has_value()) {
    SUCCEED("platform does not expose CPU time counters");
    return;
  }

  CHECK(first->total_ticks > 0);
  CHECK(first->idle_ticks <= first->total_ticks);

  const auto second = host->query_cpu_times();
  REQUIRE(second.has_value());
  CHECK(second->total_ticks >= first->total_ticks);
  CHECK(second->idle_ticks >= first->idle_ticks);
}
