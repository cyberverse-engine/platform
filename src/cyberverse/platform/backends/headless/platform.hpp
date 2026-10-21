/**
 * @file platform.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Headless platform
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <memory>

#include "cyberverse/platform/backends/headless/window.hpp"
#include "cyberverse/platform/platform.hpp"

namespace cyberverse {
namespace platform {
namespace backends {
namespace headless {

/**
 * @brief Headless platform implementation
 *
 * There is no native event loop: windows only report the events that were
 * injected into them. Timers use the platform-independent steady clock.
 *
 */
class platform final : public cyberverse::platform::platform {
public:
  platform() = default;

  ~platform() override = default;

  cyberverse::platform::backend_type backend_type() const noexcept override;

  std::shared_ptr<cyberverse::platform::timer> create_timer() override;

  std::shared_ptr<cyberverse::platform::window> create_window(const window_config& config) override;
};

} // namespace headless
} // namespace backends
} // namespace platform
} // namespace cyberverse
