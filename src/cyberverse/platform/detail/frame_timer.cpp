/**
 * @file frame_timer.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Monotonic frame timer implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/detail/frame_timer.hpp"

#include <utility>

namespace cyberverse::platform::detail {

frame_timer::frame_timer(std::function<void()> release_handler) : release_(std::move(release_handler)) {}

void frame_timer::tick() noexcept {
  const auto now = clock::now();
  if (!started_) {
    started_ = true;
    start_ = now;
    last_ = now;
  }

  delta_seconds_ = std::chrono::duration<double>(now - last_).count();
  total_seconds_ = std::chrono::duration<double>(now - start_).count();
  last_ = now;
  ++frame_count_;
}

double frame_timer::delta_seconds() const noexcept { return delta_seconds_; }

double frame_timer::total_seconds() const noexcept { return total_seconds_; }

std::uint64_t frame_timer::frame_count() const noexcept { return frame_count_; }

} // namespace cyberverse::platform::detail
