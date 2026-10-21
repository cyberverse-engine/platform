/**
 * @file platform.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Platform
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <memory>
#include <span>

#include "cyberverse/platform/backend.hpp"
#include "cyberverse/platform/timer.hpp"
#include "cyberverse/platform/window.hpp"

namespace cyberverse {
namespace platform {

/**
 * @brief Platform
 *
 * A platform is the entry point for windows, timers, and the backend-specific
 * event loop. Objects created by a platform keep it alive through a release
 * handler, so the platform is always released after its last window and timer.
 *
 * The headless backend is always built. The GLFW backend is built when
 * CYBERVERSE_PLATFORM_BACKEND_GLFW is enabled. SDL is reserved but not
 * implemented; enabling it is a configure error.
 *
 */
class platform : public std::enable_shared_from_this<platform> {
public:
  virtual ~platform() = default;

  /**
   * @brief Get the backend type object
   *
   * @return backend_type
   */
  [[nodiscard]] virtual backend_type backend_type() const noexcept = 0;

  /**
   * @brief Create a timer object
   *
   *  NOTE: timer object will hold a release callback function which will hold a shared pointer of self. So, this will
   *  guarantee that platform object is always be released after all timer objects are released.
   *
   * @return std::shared_ptr<timer>
   */
  virtual std::shared_ptr<timer> create_timer() = 0;

  /**
   * @brief Create a window object
   *
   *  NOTE: window object will hold a release callback function which will hold a shared pointer of self. So, this will
   *  guarantee that platform object is always be released after all window objects are released.
   *
   * @param config Window config
   * @return std::shared_ptr<window>
   */
  virtual std::shared_ptr<window> create_window(const window_config& config) = 0;
};

/**
 * @brief Make a new platform
 *
 * @param type Backend type
 * @return std::shared_ptr<platform>
 * @throws std::invalid_argument If the backend was not built (GLFW disabled) or
 *         is not implemented (SDL).
 * @throws std::system_error If the backend cannot be initialized.
 */
std::shared_ptr<platform> make_platform(backend_type type);

/**
 * @brief Default backend configured in compile time
 *
 * GLFW when it was built, otherwise headless.
 *
 * @return backend_type
 */
backend_type default_backend() noexcept;

/**
 * @brief All supported backends
 *
 * The returned list always contains headless and contains GLFW only when it was
 * built.
 *
 * @return std::span<backend_type>
 */
std::span<const backend_type> supported_backends() noexcept;

} // namespace platform
} // namespace cyberverse
