/**
 * @file window.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief macOS native window helpers
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <GLFW/glfw3.h>

namespace cyberverse {
namespace platform {
namespace backends {
namespace glfw {

/**
 * @brief Get the NSView of a GLFW window
 *
 * The view is configured as a layer-backed view whose layer is a CAMetalLayer,
 * which MoltenVK needs to create a Metal-backed Vulkan surface.
 *
 * @param window GLFW window
 * @return void* NSView*
 */
void* get_ns_window_view(GLFWwindow* window) noexcept;

/**
 * @brief Get the CAMetalLayer installed on a GLFW window
 *
 * @param window GLFW window
 * @return void* CAMetalLayer*
 */
void* get_metal_layer(GLFWwindow* window) noexcept;

} // namespace glfw
} // namespace backends
} // namespace platform
} // namespace cyberverse
