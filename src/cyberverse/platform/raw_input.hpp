/**
 * @file raw_input.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Raw input
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <bitset>
#include <cstddef>
#include <string>
#include <string_view>

#include <glm/vec2.hpp>

namespace cyberverse {
namespace platform {

/**
 * @brief Key id (exclude mod keys)
 *
 * Values 0 - 7 are mouse buttons so that a single key mapping can bind keyboard
 * keys and mouse buttons.
 *
 */
enum class key_id : int {
  unknown = 1,
  // Mouse button, from 0 - 7
  mouse_button1 = 0,
  mouse_button2 = 1,
  mouse_button3 = 2,
  mouse_button4 = 3,
  mouse_button5 = 4,
  mouse_button6 = 5,
  mouse_button7 = 6,
  mouse_button8 = 7,
  mouse_last = mouse_button8,
  mouse_left = mouse_button1,
  mouse_right = mouse_button2,
  mouse_middle = mouse_button3,
  // Keyboard keys
  // Printable keys
  key_space = 32,
  key_apostrophe = 39, /* ' */
  key_comma = 44,      /* , */
  key_minus = 45,      /* - */
  key_period = 46,     /* . */
  key_slash = 47,      /* / */
  key_0 = 48,
  key_1 = 49,
  key_2 = 50,
  key_3 = 51,
  key_4 = 52,
  key_5 = 53,
  key_6 = 54,
  key_7 = 55,
  key_8 = 56,
  key_9 = 57,
  key_semicolon = 59, /* ; */
  key_equal = 61,     /* = */
  key_a = 65,
  key_b = 66,
  key_c = 67,
  key_d = 68,
  key_e = 69,
  key_f = 70,
  key_g = 71,
  key_h = 72,
  key_i = 73,
  key_j = 74,
  key_k = 75,
  key_l = 76,
  key_m = 77,
  key_n = 78,
  key_o = 79,
  key_p = 80,
  key_q = 81,
  key_r = 82,
  key_s = 83,
  key_t = 84,
  key_u = 85,
  key_v = 86,
  key_w = 87,
  key_x = 88,
  key_y = 89,
  key_z = 90,
  key_left_bracket = 91,  /* [ */
  key_backslash = 92,     /* \ */
  key_right_bracket = 93, /* ] */
  key_grave_accent = 96,  /* ` */
  key_world_1 = 161,      /* non-us #1 */
  key_world_2 = 162,      /* non-us #2 */
  // function keys
  key_escape = 256,
  key_enter = 257,
  key_tab = 258,
  key_backspace = 259,
  key_insert = 260,
  key_delete = 261,
  key_right = 262,
  key_left = 263,
  key_down = 264,
  key_up = 265,
  key_page_up = 266,
  key_page_down = 267,
  key_home = 268,
  key_end = 269,
  key_caps_lock = 280,
  key_scroll_lock = 281,
  key_num_lock = 282,
  key_print_screen = 283,
  key_pause = 284,
  key_f1 = 290,
  key_f2 = 291,
  key_f3 = 292,
  key_f4 = 293,
  key_f5 = 294,
  key_f6 = 295,
  key_f7 = 296,
  key_f8 = 297,
  key_f9 = 298,
  key_f10 = 299,
  key_f11 = 300,
  key_f12 = 301,
  key_f13 = 302,
  key_f14 = 303,
  key_f15 = 304,
  key_f16 = 305,
  key_f17 = 306,
  key_f18 = 307,
  key_f19 = 308,
  key_f20 = 309,
  key_f21 = 310,
  key_f22 = 311,
  key_f23 = 312,
  key_f24 = 313,
  key_f25 = 314,
  key_kp_0 = 320,
  key_kp_1 = 321,
  key_kp_2 = 322,
  key_kp_3 = 323,
  key_kp_4 = 324,
  key_kp_5 = 325,
  key_kp_6 = 326,
  key_kp_7 = 327,
  key_kp_8 = 328,
  key_kp_9 = 329,
  key_kp_decimal = 330,
  key_kp_divide = 331,
  key_kp_multiply = 332,
  key_kp_subtract = 333,
  key_kp_add = 334,
  key_kp_enter = 335,
  key_kp_equal = 336,
  key_left_shift = 340,
  key_left_control = 341,
  key_left_alt = 342,
  key_left_super = 343,
  key_right_shift = 344,
  key_right_control = 345,
  key_right_alt = 346,
  key_right_super = 347,
  key_menu = 348,
  // Max value of keyboard_mouse_key_id
  max_value,
};

/**
 * @brief Mod keys
 *
 */
enum class mod_keys : int {
  none = 0,
  shift = 1 << 0,
  control = 1 << 1,
  alt = 1 << 2,   // Option on macos
  super = 1 << 3, // Windows or command
  caps_lock = 1 << 4,
  num_lock = 1 << 5,
};

inline constexpr mod_keys operator|(mod_keys a, mod_keys b) noexcept {
  return static_cast<mod_keys>(static_cast<int>(a) | static_cast<int>(b));
}

inline constexpr mod_keys& operator|=(mod_keys& a, mod_keys b) noexcept {
  a = a | b;
  return a;
}

inline constexpr mod_keys operator&(mod_keys a, mod_keys b) noexcept {
  return static_cast<mod_keys>(static_cast<int>(a) & static_cast<int>(b));
}

inline constexpr mod_keys& operator&=(mod_keys& a, mod_keys b) noexcept {
  a = a & b;
  return a;
}

inline constexpr mod_keys operator~(mod_keys a) noexcept { return static_cast<mod_keys>(~static_cast<int>(a)); }

/**
 * @brief Mouse button
 *
 * Only the buttons that have a portable name are listed here. Buttons 4 - 8 are
 * still tracked through their @ref key_id values.
 *
 */
enum class mouse_button : int { left = 0, right = 1, middle = 2 };

/**
 * @brief Raw input state
 *
 * The state is filled by a backend during @c begin_frame() ... @c poll_events()
 * and is read by the application and by @ref key_mapping.
 *
 * Frame protocol:
 *
 * 1. `poll_events()` calls `begin_frame()` first. It clears the per-frame
 *    pressed/released transitions, mouse and scroll deltas, and text input.
 * 2. Events received during the poll update the persistent down state and the
 *    per-frame transition state.
 * 3. The application queries the state for the rest of the frame.
 *
 * Methods without a const qualifier are backend-facing. They are public because
 * backends live in other namespaces, not because applications should normally
 * call them.
 *
 */
class raw_input_state {
public:
  /**
   * @brief Begin a new frame
   *
   */
  void begin_frame() noexcept;

  /**
   * @brief Release every pressed key and mouse button
   *
   * The releases are reported as released-this-frame transitions. Backends call
   * this when a window loses focus so that no key remains stuck.
   *
   */
  void release_all() noexcept;

  /**
   * @brief Is the key held down
   *
   * @param key Key id
   * @return bool
   */
  [[nodiscard]] bool is_key_down(key_id key) const noexcept;

  /**
   * @brief Did the key go down during this frame
   *
   * @param key Key id
   * @return bool
   */
  [[nodiscard]] bool is_key_pressed(key_id key) const noexcept;

  /**
   * @brief Did the key go up during this frame
   *
   * @param key Key id
   * @return bool
   */
  [[nodiscard]] bool is_key_released(key_id key) const noexcept;

  /**
   * @brief Are all modifier bits in @p mods held down
   *
   * `mod_keys::none` always returns true.
   *
   * @param mods Modifier bits
   * @return bool
   */
  [[nodiscard]] bool is_mod_down(mod_keys mods) const noexcept;

  /**
   * @brief Did all modifier bits in @p mods go down during this frame
   *
   * @param mods Modifier bits
   * @return bool
   */
  [[nodiscard]] bool is_mod_pressed(mod_keys mods) const noexcept;

  /**
   * @brief Did all modifier bits in @p mods go up during this frame
   *
   * @param mods Modifier bits
   * @return bool
   */
  [[nodiscard]] bool is_mod_released(mod_keys mods) const noexcept;

  /**
   * @brief All modifier bits that are currently held down
   *
   * @return mod_keys
   */
  [[nodiscard]] mod_keys mods_down() const noexcept;

  /**
   * @brief Is the mouse button held down
   *
   * @param button Mouse button
   * @return bool
   */
  [[nodiscard]] bool is_mouse_button_down(mouse_button button) const noexcept;

  /**
   * @brief Is the mouse button held down
   *
   * @param button Mouse button index, 0 - 7
   * @return bool
   */
  [[nodiscard]] bool is_mouse_button_down(int button) const noexcept;

  /**
   * @brief Did the mouse button go down during this frame
   *
   * @param button Mouse button
   * @return bool
   */
  [[nodiscard]] bool is_mouse_button_pressed(mouse_button button) const noexcept;

  /**
   * @brief Did the mouse button go down during this frame
   *
   * @param button Mouse button index, 0 - 7
   * @return bool
   */
  [[nodiscard]] bool is_mouse_button_pressed(int button) const noexcept;

  /**
   * @brief Did the mouse button go up during this frame
   *
   * @param button Mouse button
   * @return bool
   */
  [[nodiscard]] bool is_mouse_button_released(mouse_button button) const noexcept;

  /**
   * @brief Did the mouse button go up during this frame
   *
   * @param button Mouse button index, 0 - 7
   * @return bool
   */
  [[nodiscard]] bool is_mouse_button_released(int button) const noexcept;

  /**
   * @brief Mouse position in window content coordinates
   *
   * @return glm::vec2
   */
  [[nodiscard]] glm::vec2 mouse_position() const noexcept;

  /**
   * @brief Accumulated mouse movement during this frame
   *
   * @return glm::vec2
   */
  [[nodiscard]] glm::vec2 mouse_delta() const noexcept;

  /**
   * @brief Accumulated scroll movement during this frame
   *
   * @return glm::vec2
   */
  [[nodiscard]] glm::vec2 scroll_delta() const noexcept;

  /**
   * @brief UTF-8 text entered during this frame
   *
   * @return std::string_view
   */
  [[nodiscard]] std::string_view text_input() const noexcept;

  /**
   * @brief Mark a key as held down
   *
   * Repeated calls while the key is already down are ignored, matching key
   * auto-repeat. Modifier keys update the modifier state as well.
   *
   * @param key Key id
   */
  void key_down(key_id key) noexcept;

  /**
   * @brief Mark a key as released
   *
   * @param key Key id
   */
  void key_up(key_id key) noexcept;

  /**
   * @brief Replace the modifier state
   *
   * Caps lock and num lock are toggle states provided by the backend; they are
   * not derived from key press/release events.
   *
   * @param mods Modifier bits that are currently held or toggled on
   */
  void set_mod_keys(mod_keys mods) noexcept;

  /**
   * @brief Mark a mouse button as held down
   *
   * Also updates the matching `key_id::mouse_button*` state so that key
   * mappings can bind mouse buttons.
   *
   * @param button Mouse button
   */
  void mouse_button_down(mouse_button button) noexcept;

  /**
   * @brief Mark a mouse button as held down
   *
   * @param button Mouse button index, 0 - 7
   */
  void mouse_button_down(int button) noexcept;

  /**
   * @brief Mark a mouse button as released
   *
   * @param button Mouse button
   */
  void mouse_button_up(mouse_button button) noexcept;

  /**
   * @brief Mark a mouse button as released
   *
   * @param button Mouse button index, 0 - 7
   */
  void mouse_button_up(int button) noexcept;

  /**
   * @brief Report a new absolute mouse position
   *
   * The first position reported after construction does not produce a delta.
   *
   * @param x X coordinate
   * @param y Y coordinate
   */
  void mouse_move(float x, float y) noexcept;

  /**
   * @brief Report scroll input
   *
   * @param x Horizontal offset
   * @param y Vertical offset
   */
  void mouse_scroll(float x, float y) noexcept;

  /**
   * @brief Append one Unicode code point to the text input
   *
   * Invalid code points and surrogates are ignored.
   *
   * @param codepoint Unicode code point
   */
  void add_text_input(char32_t codepoint);

  /**
   * @brief Append UTF-8 text to the text input
   *
   * @param text UTF-8 text
   */
  void add_text_input(std::string_view text);

private:
  static constexpr std::size_t key_capacity = 512;
  static constexpr std::size_t mouse_capacity = 8;
  static constexpr std::size_t mod_capacity = 8;

  static bool valid_key(key_id key) noexcept;
  static bool valid_mouse_button(int button) noexcept;

  void set_key(key_id key, bool down) noexcept;
  void set_mouse_button(int button, bool down) noexcept;
  void set_mod_bit(mod_keys bit, bool down) noexcept;
  void update_modifier_from_key(key_id key, bool down) noexcept;

  std::bitset<key_capacity> keys_down_;
  std::bitset<key_capacity> keys_pressed_this_frame_;
  std::bitset<key_capacity> keys_released_this_frame_;

  std::bitset<mod_capacity> mod_keys_down_;
  std::bitset<mod_capacity> mod_keys_pressed_this_frame_;
  std::bitset<mod_capacity> mod_keys_released_this_frame_;

  glm::vec2 mouse_position_{ 0.0f };
  glm::vec2 mouse_delta_{ 0.0f };
  glm::vec2 scroll_delta_{ 0.0f };
  bool has_mouse_position_ = false;

  std::bitset<mouse_capacity> mouse_buttons_down_;
  std::bitset<mouse_capacity> mouse_buttons_pressed_this_frame_;
  std::bitset<mouse_capacity> mouse_buttons_released_this_frame_;

  std::string text_input_;
};

} // namespace platform
} // namespace cyberverse
