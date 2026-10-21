#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <system_error>

#include "cyberverse/platform.hpp"

using namespace cyberverse::platform;

namespace {

bool contains(backend_type type) {
  const auto backends = supported_backends();
  return std::find(backends.begin(), backends.end(), type) != backends.end();
}

} // namespace

TEST_CASE("supported backends include headless and the default", "[platform]") {
  const auto backends = supported_backends();
  REQUIRE_FALSE(backends.empty());
  CHECK(contains(backend_type::headless));
  CHECK(contains(default_backend()));
  CHECK_FALSE(contains(backend_type::sdl));
}

TEST_CASE("backend names are human readable", "[platform]") {
  CHECK(std::string(backend_name(backend_type::glfw)) == "glfw");
  CHECK(std::string(backend_name(backend_type::sdl)) == "sdl");
  CHECK(std::string(backend_name(backend_type::headless)) == "headless");
}

TEST_CASE("headless platform can be created", "[platform][headless]") {
  auto host = make_platform(backend_type::headless);
  REQUIRE(host != nullptr);
  CHECK(host->backend_type() == backend_type::headless);
  CHECK(host->create_timer() != nullptr);
  CHECK(host->create_window(window_config{}) != nullptr);
}

TEST_CASE("the SDL backend reports that it is not implemented", "[platform][sdl]") {
  CHECK_THROWS_AS(make_platform(backend_type::sdl), std::invalid_argument);
}

TEST_CASE("an unknown backend is rejected", "[platform]") {
  CHECK_THROWS_AS(make_platform(static_cast<backend_type>(99)), std::invalid_argument);
}

#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW
TEST_CASE("the GLFW backend is listed and can be created", "[platform][glfw]") {
  CHECK(contains(backend_type::glfw));

  try {
    auto host = make_platform(backend_type::glfw);
    REQUIRE(host != nullptr);
    CHECK(host->backend_type() == backend_type::glfw);
  } catch (const std::system_error& error) {
    WARN("GLFW is unavailable in this environment: " << error.what());
    SUCCEED("GLFW initialization is environment dependent");
  }
}
#else
TEST_CASE("the GLFW backend is not listed when it is disabled", "[platform][glfw]") {
  CHECK_FALSE(contains(backend_type::glfw));
  CHECK_THROWS_AS(make_platform(backend_type::glfw), std::invalid_argument);
}
#endif
