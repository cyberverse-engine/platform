/**
 * @file window.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Headless window
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "cyberverse/platform/detail/release_guard.hpp"
#include "cyberverse/platform/window.hpp"

namespace cyberverse {
namespace platform {
namespace backends {
namespace headless {

/**
 * @brief Window release handler type
 *
 */
using window_release_handler = std::function<void()>;

/**
 * @brief Headless window implementation
 *
 * The headless window has no native handle and a framebuffer scale of (1, 1).
 * Input and resize events can be injected for deterministic tests. Injected
 * events are queued and dispatched by the next @ref poll_events call, exactly
 * like native events.
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

  /**
   * @brief Queue a key press
   *
   * @param key Key id
   */
  void inject_key_down(key_id key);

  /**
   * @brief Queue a key release
   *
   * @param key Key id
   */
  void inject_key_up(key_id key);

  /**
   * @brief Queue a mouse button press
   *
   * @param button Mouse button
   */
  void inject_mouse_button_down(mouse_button button);

  /**
   * @brief Queue a mouse button release
   *
   * @param button Mouse button
   */
  void inject_mouse_button_up(mouse_button button);

  /**
   * @brief Queue an absolute mouse position
   *
   * @param x X coordinate
   * @param y Y coordinate
   */
  void inject_mouse_move(float x, float y);

  /**
   * @brief Queue scroll input
   *
   * @param x Horizontal offset
   * @param y Vertical offset
   */
  void inject_mouse_scroll(float x, float y);

  /**
   * @brief Queue a modifier state update
   *
   * Uses the same semantics as @ref raw_input_state::set_mod_keys.
   *
   * @param mods Modifier bits
   */
  void inject_mod_keys(mod_keys mods);

  /**
   * @brief Queue text input
   *
   * @param codepoint Unicode code point
   */
  void inject_text(char32_t codepoint);

  /**
   * @brief Queue UTF-8 text input
   *
   * @param text UTF-8 text
   */
  void inject_text(std::string_view text);

  /**
   * @brief Queue a focus loss
   *
   * Releases every held key and mouse button on the next poll.
   *
   */
  void inject_focus_lost();

  /**
   * @brief Queue a resize
   *
   * Updates the size and invokes the resize callback on the next poll.
   *
   * @param width New width in pixels
   * @param height New height in pixels
   */
  void inject_resize(int width, int height);

private:
  void queue(std::function<void()> event);

  detail::release_guard release_;
  native_window_handle native_handle_{};
  raw_input_state input_;
  window_resize_event_handler resize_callback_;
  cursor_mode cursor_mode_ = cursor_mode::normal;
  std::vector<std::function<void()>> pending_events_;
  int width_ = 0;
  int height_ = 0;
  bool should_close_ = false;
};

} // namespace headless
} // namespace backends
} // namespace platform
} // namespace cyberverse
