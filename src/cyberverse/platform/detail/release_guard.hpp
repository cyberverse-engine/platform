/**
 * @file release_guard.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Platform lifetime release guard
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <utility>

namespace cyberverse {
namespace platform {
namespace detail {

/**
 * @brief Run a release handler when the guard is destroyed
 *
 * Windows and timers own one of these guards. The handler holds a shared
 * pointer to the platform, which keeps the platform alive for as long as any
 * child object exists.
 *
 * The handler is invoked at most once. Exceptions thrown by the handler are
 * swallowed because the guard is destroyed from a destructor.
 *
 */
class release_guard {
public:
  release_guard() noexcept = default;

  explicit release_guard(std::function<void()> handler) noexcept : handler_(std::move(handler)) {}

  ~release_guard() { release(); }

  release_guard(const release_guard&) = delete;
  release_guard(release_guard&&) = delete;

  release_guard& operator=(const release_guard&) = delete;
  release_guard& operator=(release_guard&&) = delete;

  /**
   * @brief Invoke the handler, if it has not been invoked yet
   *
   */
  void release() noexcept {
    if (!handler_) {
      return;
    }

    auto handler = std::move(handler_);
    handler_ = nullptr;
    try {
      handler();
    } catch (...) {
      // A release handler only drops a shared pointer; never propagate from a
      // destructor path.
    }
  }

  /**
   * @brief Is a handler still pending
   *
   * @return bool
   */
  explicit operator bool() const noexcept { return static_cast<bool>(handler_); }

private:
  std::function<void()> handler_;
};

} // namespace detail
} // namespace platform
} // namespace cyberverse
