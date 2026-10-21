/**
 * @file native.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW native window handle query
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <GLFW/glfw3.h>

#include "cyberverse/platform/window.hpp"

namespace cyberverse {
namespace platform {
namespace backends {
namespace glfw {

/**
 * @brief Query the native handle of a GLFW window
 *
 * The macOS implementation lives in `macos/window.mm` because it also installs
 * a CAMetalLayer on the content view. Other platforms use `native.cpp`.
 *
 * @param window GLFW window
 * @return native_window_handle
 */
cyberverse::platform::native_window_handle query_native_handle(GLFWwindow* window) noexcept;

} // namespace glfw
} // namespace backends
} // namespace platform
} // namespace cyberverse
