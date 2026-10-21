/**
 * @file platform.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW platform
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <memory>

#include "cyberverse/platform/backends/glfw/glfw.hpp"
#include "cyberverse/platform/platform.hpp"

namespace cyberverse {
namespace platform {
namespace backends {
namespace glfw {

/**
 * @brief Window release handler type
 *
 */
using window_release_handler = std::function<void()>;

/**
 * @brief GLFW platform implementation
 *
 * Constructing the platform initializes GLFW. The process-wide GLFW library is
 * reference counted, so several platform objects can coexist; it is terminated
 * after the last platform is destroyed.
 *
 */
class platform final : public cyberverse::platform::platform {
public:
  platform();

  ~platform() override;

  cyberverse::platform::backend_type backend_type() const noexcept override;

  std::shared_ptr<cyberverse::platform::timer> create_timer() override;

  std::shared_ptr<cyberverse::platform::window> create_window(const window_config& config) override;
};

} // namespace glfw
} // namespace backends
} // namespace platform
} // namespace cyberverse
