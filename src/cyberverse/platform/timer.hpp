/**
 * @file timer.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Time
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>

namespace cyberverse {
namespace platform {

/**
 * @brief Frame timer
 *
 * A timer measures wall-clock time with a monotonic clock. The application owns
 * the frame loop; it calls @ref tick once at the start of each frame and reads
 * the accumulated values afterwards.
 *
 * Timers created by @ref platform::create_timer keep the platform alive through
 * a release handler, so a timer can safely be used after the caller drops its
 * last reference to the platform.
 *
 */
class timer {
public:
  timer() = default;

  virtual ~timer();

  timer(const timer&) = delete;
  timer(timer&&) = delete;

  timer& operator=(const timer&) = delete;
  timer& operator=(timer&&) = delete;

  /**
   * @brief This method is called at start of each frame.
   *
   * The first call starts the clock and reports a zero delta. Every call
   * increments the frame count.
   *
   */
  virtual void tick() noexcept = 0;

  /**
   * @brief Seconds elapsed since the previous tick
   *
   * @return double
   */
  virtual double delta_seconds() const noexcept = 0;

  /**
   * @brief Seconds elapsed since the first tick
   *
   * @return double
   */
  virtual double total_seconds() const noexcept = 0;

  /**
   * @brief Number of completed ticks
   *
   * @return uint64_t
   */
  virtual uint64_t frame_count() const noexcept = 0;
};

} // namespace platform
} // namespace cyberverse
