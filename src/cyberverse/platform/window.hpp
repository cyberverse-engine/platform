/**
 * @file window.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Window
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <string>

#include <glm/vec2.hpp>

#include "cyberverse/platform/backend.hpp"
#include "cyberverse/platform/raw_input.hpp"

namespace cyberverse {
namespace platform {

/**
 * @brief Cursor mode
 *
 */
enum class cursor_mode { normal, hidden, disabled };

/**
 * @brief Window config
 *
 */
struct window_config {
  int width = 1280;             // Client area width in screen coordinates
  int height = 720;             // Client area height in screen coordinates
  std::string title = "Window"; // Window title
  bool resizable = true;        // Allow the user to resize the window
  bool vsync = true;            // Hint for the renderer's swap chain
};

/**
 * @brief Native window handle
 *
 * Fields that do not apply to a backend or platform stay null.
 *
 */
struct native_window_handle {
  backend_type backend = backend_type::headless; // Backend type
  void* window = nullptr;                        // HWND / X11 Window / NSWindow*
  void* display = nullptr;                       // X11 Display* / Wayland wl_display*
  void* surface = nullptr;                       // Wayland wl_surface* / CAMetalLayer*
  void* view = nullptr;                          // NSView* (MoltenVK/CAMetalLayer host)
};

/**
 * @brief Resize event handler
 *
 * The callback receives the new framebuffer size in pixels.
 *
 */
using window_resize_event_handler = std::function<void(int, int)>;

/**
 * @brief Window base class
 *
 * A window owns its input state and keeps the platform alive through a release
 * handler until the window is destroyed.
 *
 * Frame protocol: call @ref poll_events once per frame before reading input.
 * `poll_events()` starts a new input frame and then dispatches platform events.
 *
 * Unless stated otherwise, windows must be created, polled, and destroyed on
 * the main thread.
 *
 */
class window {
public:
  window() = default;

  virtual ~window();

  window(const window&) = delete;
  window(window&&) = delete;

  window& operator=(const window&) = delete;
  window& operator=(window&&) = delete;

  /**
   * @brief Get backend type
   *
   * @return backend_type
   */
  virtual backend_type backend_type() const noexcept = 0;

  /**
   * @brief Should close or not
   *
   * @return bool
   */
  virtual bool should_close() const noexcept = 0;

  /**
   * @brief Request close
   *
   */
  virtual void request_close() noexcept = 0;

  /**
   * @brief Poll events
   *
   * Starts a new input frame and dispatches the events that arrived since the
   * previous call.
   *
   */
  virtual void poll_events() noexcept = 0;

  /**
   * @brief Native window handle
   *
   * @return const native_window_handle&
   */
  [[nodiscard]] virtual const native_window_handle& native_handle() const noexcept = 0;

  /**
   * @brief Framebuffer width in pixels
   *
   * @return int
   */
  [[nodiscard]] virtual int width() const noexcept = 0;

  /**
   * @brief Framebuffer height in pixels
   *
   * @return int
   */
  [[nodiscard]] virtual int height() const noexcept = 0;

  /**
   * @brief Framebuffer scale
   *
   * Ratio between the framebuffer size and the window size in screen
   * coordinates. It is (1, 1) on non-HiDPI displays and on headless windows.
   *
   * @return glm::vec2
   */
  [[nodiscard]] virtual glm::vec2 framebuffer_scale() const noexcept = 0;

  /**
   * @brief Set the cursor mode
   *
   * @param mode Cursor mode
   */
  virtual void set_cursor_mode(cursor_mode mode) noexcept = 0;

  /**
   * @brief Get the Cursor Mode object
   *
   * @return cursor_mode
   */
  [[nodiscard]] virtual cursor_mode get_cursor_mode() const noexcept = 0;

  /**
   * @brief Get the resize callback object
   *
   * @return window_resize_event_handler
   */
  [[nodiscard]] virtual window_resize_event_handler get_resize_callback() const noexcept = 0;

  /**
   * @brief Set the resize callback object
   *
   * @param handler Handler
   */
  virtual void set_resize_callback(window_resize_event_handler handler) noexcept = 0;

  /**
   * @brief Get input state
   *
   * @return const raw_input_state&
   */
  [[nodiscard]] virtual const raw_input_state& input_state() const noexcept = 0;
};

} // namespace platform
} // namespace cyberverse
