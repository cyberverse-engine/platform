/**
 * @file native.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW native window handle query for non-Apple platforms
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/backends/glfw/native.hpp"

#if !defined(__APPLE__)

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32 1
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11 1
#define GLFW_EXPOSE_NATIVE_WAYLAND 1
#endif

#include <GLFW/glfw3native.h>

#include <cstdint>

namespace cyberverse::platform::backends::glfw {

cyberverse::platform::native_window_handle query_native_handle(GLFWwindow* window) noexcept {
  cyberverse::platform::native_window_handle handle;
  handle.backend = cyberverse::platform::backend_type::glfw;
  if (window == nullptr) {
    return handle;
  }

#if defined(_WIN32)
  handle.window = static_cast<void*>(glfwGetWin32Window(window));
#elif defined(__linux__)
  if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
    handle.display = static_cast<void*>(glfwGetX11Display());
    handle.window = reinterpret_cast<void*>(static_cast<std::uintptr_t>(glfwGetX11Window(window)));
  } else if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
    handle.display = glfwGetWaylandDisplay();
    handle.surface = glfwGetWaylandWindow(window);
  }
#endif

  return handle;
}

} // namespace cyberverse::platform::backends::glfw

#endif // !defined(__APPLE__)
