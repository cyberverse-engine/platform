/**
 * @file platform.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW platform implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/backends/glfw/platform.hpp"

#include <mutex>
#include <system_error>
#include <utility>

#include "cyberverse/platform/backends/glfw/window.hpp"
#include "cyberverse/platform/detail/frame_timer.hpp"

namespace cyberverse::platform::backends::glfw {

namespace {

std::mutex& instance_mutex() {
  static std::mutex mutex;
  return mutex;
}

int& instance_count() {
  static int count = 0;
  return count;
}

void acquire_glfw() {
  const std::lock_guard<std::mutex> lock(instance_mutex());
  if (instance_count() == 0 && glfwInit() != GLFW_TRUE) {
    throw std::system_error(make_error_code(), "failed to initialize GLFW");
  }
  ++instance_count();
}

void release_glfw() noexcept {
  const std::lock_guard<std::mutex> lock(instance_mutex());
  if (instance_count() == 0) {
    return;
  }
  --instance_count();
  if (instance_count() == 0) {
    glfwTerminate();
  }
}

} // namespace

platform::platform() { acquire_glfw(); }

platform::~platform() { release_glfw(); }

cyberverse::platform::backend_type platform::backend_type() const noexcept {
  return cyberverse::platform::backend_type::glfw;
}

std::shared_ptr<cyberverse::platform::timer> platform::create_timer() {
  auto keep_alive = shared_from_this();
  return std::make_shared<detail::frame_timer>([keep_alive = std::move(keep_alive)]() {
    (void)keep_alive;
  });
}

std::shared_ptr<cyberverse::platform::window> platform::create_window(const window_config& config) {
  auto keep_alive = shared_from_this();
  return std::make_shared<window>(config, [keep_alive = std::move(keep_alive)]() {
    (void)keep_alive;
  });
}

} // namespace cyberverse::platform::backends::glfw
