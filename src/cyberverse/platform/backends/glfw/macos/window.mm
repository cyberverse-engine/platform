/**
 * @file window.mm
 * @author simon2077s (simon2077s@gmail.com)
 * @brief macOS native window helpers
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */

#define GLFW_EXPOSE_NATIVE_COCOA 1

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <QuartzCore/QuartzCore.h>

#include "cyberverse/platform/backends/glfw/macos/window.hpp"
#include "cyberverse/platform/backends/glfw/native.hpp"

namespace cyberverse::platform::backends::glfw {

namespace {

// MoltenVK implements the Vulkan API on top of Metal. To render into a GLFW
// window we have to install a CAMetalLayer on its content view. glfwCreateWindowSurface
// does the same internally, but DiligentEngine creates the Vulkan surface inside
// its own ISwapChain implementation and cannot call the GLFW helper.
CAMetalLayer* install_metal_layer(GLFWwindow* window) noexcept {
  if (window == nullptr) {
    return nil;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  if (ns_window == nil) {
    return nil;
  }

  NSView* view = [ns_window contentView];
  if (view == nil) {
    return nil;
  }

  CAMetalLayer* layer = [CAMetalLayer layer];
  [view setLayer:layer];
  [view setWantsLayer:YES];

  // Match the layer's drawable size to the backing store scale so that the
  // renderer receives retina-sized drawables.
  const CGSize view_scale = [view convertSizeToBacking:CGSizeMake(1.0, 1.0)];
  [layer setContentsScale:MIN(view_scale.width, view_scale.height)];

  return layer;
}

CAMetalLayer* ensure_metal_layer(GLFWwindow* window) noexcept {
  if (window == nullptr) {
    return nil;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  if (ns_window == nil) {
    return nil;
  }

  NSView* view = [ns_window contentView];
  if (view == nil) {
    return nil;
  }

  id layer = [view layer];
  if ([layer isKindOfClass:[CAMetalLayer class]]) {
    return static_cast<CAMetalLayer*>(layer);
  }
  return install_metal_layer(window);
}

} // namespace

void* get_ns_window_view(GLFWwindow* window) noexcept {
  if (window == nullptr) {
    return nullptr;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  if (ns_window == nil) {
    return nullptr;
  }

  NSView* view = [ns_window contentView];
  if (view == nil) {
    return nullptr;
  }

  ensure_metal_layer(window);
  return (__bridge void*)view;
}

void* get_metal_layer(GLFWwindow* window) noexcept { return (__bridge void*)ensure_metal_layer(window); }

cyberverse::platform::native_window_handle query_native_handle(GLFWwindow* window) noexcept {
  cyberverse::platform::native_window_handle handle;
  handle.backend = cyberverse::platform::backend_type::glfw;
  if (window == nullptr) {
    return handle;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  handle.window = (__bridge void*)ns_window;
  handle.view = get_ns_window_view(window);
  handle.surface = get_metal_layer(window);
  return handle;
}

} // namespace cyberverse::platform::backends::glfw
