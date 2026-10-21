/**
 * @file platform.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Headless platform implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/backends/headless/platform.hpp"

#include <utility>

#include "cyberverse/platform/detail/frame_timer.hpp"

namespace cyberverse::platform::backends::headless {

cyberverse::platform::backend_type platform::backend_type() const noexcept {
  return cyberverse::platform::backend_type::headless;
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

} // namespace cyberverse::platform::backends::headless
