#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <memory>
#include <system_error>

#include "cyberverse/platform.hpp"

using namespace cyberverse::platform;

TEST_CASE("GLFW window smoke test", "[glfw][window]") {
#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW
  // Creating a real window is opt-in so that routine test runs do not open one.
  if (std::getenv("CYBERVERSE_PLATFORM_TEST_WINDOWS") == nullptr) {
    SUCCEED("set CYBERVERSE_PLATFORM_TEST_WINDOWS=1 to create a real GLFW window");
    return;
  }

  std::shared_ptr<platform> host;
  try {
    host = make_platform(backend_type::glfw);
  } catch (const std::system_error& error) {
    WARN("GLFW is unavailable in this environment: " << error.what());
    SUCCEED("GLFW initialization is environment dependent");
    return;
  }
  REQUIRE(host != nullptr);

  std::shared_ptr<window> window;
  try {
    window_config config;
    config.width = 320;
    config.height = 240;
    config.title = "cyberverse platform test";
    window = host->create_window(config);
  } catch (const std::system_error& error) {
    WARN("cannot create a GLFW window in this environment: " << error.what());
    SUCCEED("GLFW window creation is environment dependent");
    return;
  }
  REQUIRE(window != nullptr);

  CHECK(window->backend_type() == backend_type::glfw);
  CHECK(window->width() > 0);
  CHECK(window->height() > 0);
  CHECK(window->framebuffer_scale().x > 0.0f);
  CHECK(window->framebuffer_scale().y > 0.0f);
  CHECK(window->native_handle().backend == backend_type::glfw);
#if defined(__APPLE__)
  CHECK(window->native_handle().window != nullptr);
  CHECK(window->native_handle().view != nullptr);
  CHECK(window->native_handle().surface != nullptr);
#endif

  window->set_cursor_mode(cursor_mode::hidden);
  CHECK(window->get_cursor_mode() == cursor_mode::hidden);

  window->poll_events();
  CHECK_FALSE(window->should_close());

  window->request_close();
  CHECK(window->should_close());

  window.reset();
  host.reset();
#else
  SUCCEED("the GLFW backend is not built");
#endif
}
