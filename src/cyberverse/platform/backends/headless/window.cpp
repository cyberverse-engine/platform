/**
 * @file window.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Headless window implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/backends/headless/window.hpp"

#include <utility>

namespace cyberverse::platform::backends::headless {

window::window(const window_config& config, window_release_handler release_callback)
    : release_(std::move(release_callback)), width_(config.width), height_(config.height) {
  native_handle_.backend = cyberverse::platform::backend_type::headless;
}

window::~window() = default;

cyberverse::platform::backend_type window::backend_type() const noexcept {
  return cyberverse::platform::backend_type::headless;
}

bool window::should_close() const noexcept { return should_close_; }

void window::request_close() noexcept { should_close_ = true; }

void window::poll_events() noexcept {
  input_.begin_frame();

  std::vector<std::function<void()>> pending;
  pending.swap(pending_events_);
  for (auto& event : pending) {
    try {
      event();
    } catch (...) {
      // poll_events is noexcept; an injected callback must not terminate the
      // application.
    }
  }
}

const native_window_handle& window::native_handle() const noexcept { return native_handle_; }

int window::width() const noexcept { return width_; }

int window::height() const noexcept { return height_; }

glm::vec2 window::framebuffer_scale() const noexcept { return glm::vec2{ 1.0f, 1.0f }; }

void window::set_cursor_mode(cursor_mode mode) noexcept { cursor_mode_ = mode; }

cursor_mode window::get_cursor_mode() const noexcept { return cursor_mode_; }

window_resize_event_handler window::get_resize_callback() const noexcept { return resize_callback_; }

void window::set_resize_callback(window_resize_event_handler handler) noexcept {
  resize_callback_ = std::move(handler);
}

const raw_input_state& window::input_state() const noexcept { return input_; }

void window::inject_key_down(key_id key) {
  queue([this, key] {
    input_.key_down(key);
  });
}

void window::inject_key_up(key_id key) {
  queue([this, key] {
    input_.key_up(key);
  });
}

void window::inject_mouse_button_down(mouse_button button) {
  queue([this, button] {
    input_.mouse_button_down(button);
  });
}

void window::inject_mouse_button_up(mouse_button button) {
  queue([this, button] {
    input_.mouse_button_up(button);
  });
}

void window::inject_mouse_move(float x, float y) {
  queue([this, x, y] {
    input_.mouse_move(x, y);
  });
}

void window::inject_mouse_scroll(float x, float y) {
  queue([this, x, y] {
    input_.mouse_scroll(x, y);
  });
}

void window::inject_mod_keys(mod_keys mods) {
  queue([this, mods] {
    input_.set_mod_keys(mods);
  });
}

void window::inject_text(char32_t codepoint) {
  queue([this, codepoint] {
    input_.add_text_input(codepoint);
  });
}

void window::inject_text(std::string_view text) {
  std::string owned(text);
  queue([this, text = std::move(owned)] {
    input_.add_text_input(text);
  });
}

void window::inject_focus_lost() {
  queue([this] {
    input_.release_all();
  });
}

void window::inject_resize(int width, int height) {
  queue([this, width, height] {
    width_ = width;
    height_ = height;
    if (resize_callback_) {
      resize_callback_(width_, height_);
    }
  });
}

void window::queue(std::function<void()> event) { pending_events_.push_back(std::move(event)); }

} // namespace cyberverse::platform::backends::headless
