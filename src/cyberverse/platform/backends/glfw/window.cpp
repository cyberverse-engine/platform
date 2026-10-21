/**
 * @file window.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW window implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/backends/glfw/window.hpp"

#include <system_error>
#include <utility>

namespace cyberverse::platform::backends::glfw {

static_assert(static_cast<int>(mod_keys::shift) == GLFW_MOD_SHIFT);
static_assert(static_cast<int>(mod_keys::control) == GLFW_MOD_CONTROL);
static_assert(static_cast<int>(mod_keys::alt) == GLFW_MOD_ALT);
static_assert(static_cast<int>(mod_keys::super) == GLFW_MOD_SUPER);
static_assert(static_cast<int>(mod_keys::caps_lock) == GLFW_MOD_CAPS_LOCK);
static_assert(static_cast<int>(mod_keys::num_lock) == GLFW_MOD_NUM_LOCK);

static_assert(static_cast<int>(key_id::mouse_left) == GLFW_MOUSE_BUTTON_LEFT);
static_assert(static_cast<int>(key_id::mouse_right) == GLFW_MOUSE_BUTTON_RIGHT);
static_assert(static_cast<int>(key_id::mouse_middle) == GLFW_MOUSE_BUTTON_MIDDLE);

window::window(const window_config& config, window_release_handler release_callback)
    : release_(std::move(release_callback)) {
  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

  handle_ = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
  if (handle_ == nullptr) {
    throw std::system_error(make_error_code(), "failed to create GLFW window");
  }

  glfwSetWindowUserPointer(handle_, this);
  install_callbacks();
  refresh_size();
  native_handle_ = query_native_handle(handle_);
}

window::~window() {
  if (handle_ != nullptr) {
    glfwSetWindowUserPointer(handle_, nullptr);
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
  }
  release_.release();
}

cyberverse::platform::backend_type window::backend_type() const noexcept {
  return cyberverse::platform::backend_type::glfw;
}

bool window::should_close() const noexcept { return handle_ != nullptr && glfwWindowShouldClose(handle_) == GLFW_TRUE; }

void window::request_close() noexcept {
  if (handle_ != nullptr) {
    glfwSetWindowShouldClose(handle_, GLFW_TRUE);
  }
}

void window::poll_events() noexcept {
  if (handle_ == nullptr) {
    return;
  }

  input_.begin_frame();
  glfwPollEvents();
}

const native_window_handle& window::native_handle() const noexcept { return native_handle_; }

int window::width() const noexcept { return width_; }

int window::height() const noexcept { return height_; }

glm::vec2 window::framebuffer_scale() const noexcept { return framebuffer_scale_; }

void window::set_cursor_mode(cursor_mode mode) noexcept {
  cursor_mode_ = mode;
  apply_cursor_mode();
}

cursor_mode window::get_cursor_mode() const noexcept { return cursor_mode_; }

window_resize_event_handler window::get_resize_callback() const noexcept { return resize_callback_; }

void window::set_resize_callback(window_resize_event_handler handler) noexcept {
  resize_callback_ = std::move(handler);
}

const raw_input_state& window::input_state() const noexcept { return input_; }

void window::key_callback(GLFWwindow* handle, int key, int scancode, int action, int mods) {
  (void)scancode;

  auto* owner = self(handle);
  if (owner == nullptr) {
    return;
  }

  owner->input_.set_mod_keys(static_cast<mod_keys>(mods));
  if (action == GLFW_PRESS) {
    owner->input_.key_down(static_cast<key_id>(key));
  } else if (action == GLFW_RELEASE) {
    owner->input_.key_up(static_cast<key_id>(key));
  }
}

void window::char_callback(GLFWwindow* handle, unsigned int codepoint) {
  auto* owner = self(handle);
  if (owner != nullptr) {
    owner->input_.add_text_input(static_cast<char32_t>(codepoint));
  }
}

void window::cursor_position_callback(GLFWwindow* handle, double x, double y) {
  auto* owner = self(handle);
  if (owner != nullptr) {
    owner->input_.mouse_move(static_cast<float>(x), static_cast<float>(y));
  }
}

void window::mouse_button_callback(GLFWwindow* handle, int button, int action, int mods) {
  auto* owner = self(handle);
  if (owner == nullptr) {
    return;
  }

  owner->input_.set_mod_keys(static_cast<mod_keys>(mods));
  if (action == GLFW_PRESS) {
    owner->input_.mouse_button_down(button);
  } else if (action == GLFW_RELEASE) {
    owner->input_.mouse_button_up(button);
  }
}

void window::scroll_callback(GLFWwindow* handle, double xoffset, double yoffset) {
  auto* owner = self(handle);
  if (owner != nullptr) {
    owner->input_.mouse_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
  }
}

void window::framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
  auto* owner = self(handle);
  if (owner == nullptr) {
    return;
  }

  owner->refresh_size();
  if (owner->resize_callback_) {
    try {
      owner->resize_callback_(width, height);
    } catch (...) {
      // Callbacks run inside the GLFW C event loop; never let an exception
      // escape through the C frames.
    }
  }
}

void window::window_size_callback(GLFWwindow* handle, int width, int height) {
  (void)width;
  (void)height;

  auto* owner = self(handle);
  if (owner != nullptr) {
    owner->refresh_size();
  }
}

void window::window_focus_callback(GLFWwindow* handle, int focused) {
  auto* owner = self(handle);
  if (owner != nullptr && focused == GLFW_FALSE) {
    owner->input_.release_all();
  }
}

window* window::self(GLFWwindow* handle) noexcept {
  if (handle == nullptr) {
    return nullptr;
  }
  return static_cast<window*>(glfwGetWindowUserPointer(handle));
}

void window::install_callbacks() noexcept {
  glfwSetKeyCallback(handle_, &window::key_callback);
  glfwSetCharCallback(handle_, &window::char_callback);
  glfwSetCursorPosCallback(handle_, &window::cursor_position_callback);
  glfwSetMouseButtonCallback(handle_, &window::mouse_button_callback);
  glfwSetScrollCallback(handle_, &window::scroll_callback);
  glfwSetFramebufferSizeCallback(handle_, &window::framebuffer_size_callback);
  glfwSetWindowSizeCallback(handle_, &window::window_size_callback);
  glfwSetWindowFocusCallback(handle_, &window::window_focus_callback);
}

void window::refresh_size() noexcept {
  if (handle_ == nullptr) {
    return;
  }

  int framebuffer_width = 0;
  int framebuffer_height = 0;
  int window_width = 0;
  int window_height = 0;
  glfwGetFramebufferSize(handle_, &framebuffer_width, &framebuffer_height);
  glfwGetWindowSize(handle_, &window_width, &window_height);

  width_ = framebuffer_width;
  height_ = framebuffer_height;

  if (window_width > 0 && window_height > 0 && framebuffer_width > 0 && framebuffer_height > 0) {
    framebuffer_scale_ = glm::vec2{ static_cast<float>(framebuffer_width) / static_cast<float>(window_width),
                                    static_cast<float>(framebuffer_height) / static_cast<float>(window_height) };
  } else {
    framebuffer_scale_ = glm::vec2{ 1.0f, 1.0f };
  }
}

void window::apply_cursor_mode() noexcept {
  if (handle_ == nullptr) {
    return;
  }

  int mode = GLFW_CURSOR_NORMAL;
  switch (cursor_mode_) {
  case cursor_mode::normal:
    mode = GLFW_CURSOR_NORMAL;
    break;
  case cursor_mode::hidden:
    mode = GLFW_CURSOR_HIDDEN;
    break;
  case cursor_mode::disabled:
    mode = GLFW_CURSOR_DISABLED;
    break;
  }
  glfwSetInputMode(handle_, GLFW_CURSOR, mode);

  if (glfwRawMouseMotionSupported() == GLFW_TRUE) {
    glfwSetInputMode(handle_, GLFW_RAW_MOUSE_MOTION, cursor_mode_ == cursor_mode::disabled ? GLFW_TRUE : GLFW_FALSE);
  }
}

} // namespace cyberverse::platform::backends::glfw
