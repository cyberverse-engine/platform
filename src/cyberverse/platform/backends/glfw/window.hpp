/**
 * @file window.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW window
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>

#include "cyberverse/platform/backends/glfw/glfw.hpp"
#include "cyberverse/platform/backends/glfw/native.hpp"
#include "cyberverse/platform/backends/glfw/platform.hpp"
#include "cyberverse/platform/detail/release_guard.hpp"
#include "cyberverse/platform/window.hpp"

namespace cyberverse {
namespace platform {
namespace backends {
namespace glfw {

/**
 * @brief GLFW window implementation
 *
 * The window is created without a graphics API context (GLFW_NO_API); the
 * renderer creates its own device and swap chain from @ref native_handle. On
 * macOS the content view receives a CAMetalLayer for MoltenVK.
 *
 */
class window final : public cyberverse::platform::window {
public:
  window(const window_config& config, window_release_handler release_callback);

  ~window() override;

  cyberverse::platform::backend_type backend_type() const noexcept override;

  bool should_close() const noexcept override;

  void request_close() noexcept override;

  void poll_events() noexcept override;

  const native_window_handle& native_handle() const noexcept override;

  int width() const noexcept override;

  int height() const noexcept override;

  glm::vec2 framebuffer_scale() const noexcept override;

  void set_cursor_mode(cursor_mode mode) noexcept override;

  cursor_mode get_cursor_mode() const noexcept override;

  window_resize_event_handler get_resize_callback() const noexcept override;

  void set_resize_callback(window_resize_event_handler handler) noexcept override;

  const raw_input_state& input_state() const noexcept override;

private:
  static void key_callback(GLFWwindow* handle, int key, int scancode, int action, int mods);
  static void char_callback(GLFWwindow* handle, unsigned int codepoint);
  static void cursor_position_callback(GLFWwindow* handle, double x, double y);
  static void mouse_button_callback(GLFWwindow* handle, int button, int action, int mods);
  static void scroll_callback(GLFWwindow* handle, double xoffset, double yoffset);
  static void framebuffer_size_callback(GLFWwindow* handle, int width, int height);
  static void window_size_callback(GLFWwindow* handle, int width, int height);
  static void window_focus_callback(GLFWwindow* handle, int focused);

  static window* self(GLFWwindow* handle) noexcept;

  void install_callbacks() noexcept;
  void refresh_size() noexcept;
  void apply_cursor_mode() noexcept;

  detail::release_guard release_;
  GLFWwindow* handle_ = nullptr;
  native_window_handle native_handle_{};
  raw_input_state input_;
  window_resize_event_handler resize_callback_;
  cursor_mode cursor_mode_ = cursor_mode::normal;
  glm::vec2 framebuffer_scale_{ 1.0f, 1.0f };
  int width_ = 0;
  int height_ = 0;
};

} // namespace glfw
} // namespace backends
} // namespace platform
} // namespace cyberverse
