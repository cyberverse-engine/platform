/**
 * @file frame_timer.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Monotonic frame timer
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>

#include "cyberverse/platform/detail/release_guard.hpp"
#include "cyberverse/platform/timer.hpp"

namespace cyberverse {
namespace platform {
namespace detail {

/**
 * @brief Monotonic steady-clock timer shared by the backends
 *
 */
class frame_timer final : public cyberverse::platform::timer {
public:
  explicit frame_timer(std::function<void()> release_handler);

  ~frame_timer() override = default;

  void tick() noexcept override;

  double delta_seconds() const noexcept override;

  double total_seconds() const noexcept override;

  std::uint64_t frame_count() const noexcept override;

private:
  using clock = std::chrono::steady_clock;

  release_guard release_;
  clock::time_point start_{};
  clock::time_point last_{};
  double delta_seconds_ = 0.0;
  double total_seconds_ = 0.0;
  std::uint64_t frame_count_ = 0;
  bool started_ = false;
};

} // namespace detail
} // namespace platform
} // namespace cyberverse
