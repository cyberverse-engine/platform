/**
 * @file key_mapping.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Key map
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "cyberverse/platform/raw_input.hpp"

namespace cyberverse {
namespace platform {

/**
 * @brief One physical key bound to an action
 *
 */
struct key_binding {
  key_id key = key_id::unknown;
  mod_keys modifiers = mod_keys::none;

  friend bool operator==(const key_binding&, const key_binding&) noexcept = default;
};

/**
 * @brief State of an action
 *
 */
struct action_state {
  bool down = false;     // At least one active binding is held
  bool pressed = false;  // At least one active binding went down this frame
  bool released = false; // The action went from down to up this frame
};

/**
 * @brief Key mapping type, map physical key to action.
 *
 * Bind keyboard keys and mouse buttons (through their `key_id` values) to named
 * actions. Call @ref update once per frame after polling the window, then query
 * the action state.
 *
 * Bindings require an exact modifier set: a binding with `mod_keys::shift`
 * triggers only while shift is held and control/alt/super are not. Caps lock and
 * num lock are toggle states and are ignored when matching modifiers.
 *
 * Multiple keys can be bound to the same action; the action is down while any
 * binding is active.
 *
 */
class key_mapping {
public:
  /**
   * @brief Bind a key to an action
   *
   * Binding the same key and modifier combination twice is a no-op.
   *
   * @param action Action name
   * @param key Key id
   * @param modifiers Required modifiers
   */
  void bind(std::string_view action, key_id key, mod_keys modifiers = mod_keys::none);

  /**
   * @brief Bind a mouse button to an action
   *
   * @param action Action name
   * @param button Mouse button
   * @param modifiers Required modifiers
   */
  void bind(std::string_view action, mouse_button button, mod_keys modifiers = mod_keys::none);

  /**
   * @brief Bind a key binding to an action
   *
   * @param action Action name
   * @param binding Key binding
   */
  void bind(std::string_view action, key_binding binding);

  /**
   * @brief Remove one key binding from an action
   *
   * @param action Action name
   * @param key Key id
   * @param modifiers Required modifiers
   * @return bool True when a binding was removed
   */
  bool unbind(std::string_view action, key_id key, mod_keys modifiers = mod_keys::none);

  /**
   * @brief Remove every binding of an action
   *
   * @param action Action name
   * @return bool True when the action existed
   */
  bool unbind_all(std::string_view action);

  /**
   * @brief Remove every binding and action state
   *
   */
  void clear() noexcept;

  /**
   * @brief Recompute action states from raw input
   *
   * @param state Raw input state
   */
  void update(const raw_input_state& state) noexcept;

  /**
   * @brief Is the action held down
   *
   * @param action Action name
   * @return bool
   */
  [[nodiscard]] bool is_down(std::string_view action) const noexcept;

  /**
   * @brief Did the action go down during this frame
   *
   * @param action Action name
   * @return bool
   */
  [[nodiscard]] bool is_pressed(std::string_view action) const noexcept;

  /**
   * @brief Did the action go up during this frame
   *
   * @param action Action name
   * @return bool
   */
  [[nodiscard]] bool is_released(std::string_view action) const noexcept;

  /**
   * @brief Full action state
   *
   * Unknown actions report an empty state.
   *
   * @param action Action name
   * @return action_state
   */
  [[nodiscard]] action_state state(std::string_view action) const noexcept;

  /**
   * @brief Bindings of an action
   *
   * The span stays valid until the mapping is modified. Unknown actions return
   * an empty span.
   *
   * @param action Action name
   * @return std::span<const key_binding>
   */
  [[nodiscard]] std::span<const key_binding> bindings(std::string_view action) const noexcept;

  /**
   * @brief Number of registered actions
   *
   * @return std::size_t
   */
  [[nodiscard]] std::size_t action_count() const noexcept;

private:
  struct action_hash {
    using is_transparent = void;

    [[nodiscard]] std::size_t operator()(std::string_view value) const noexcept {
      return std::hash<std::string_view>{}(value);
    }

    [[nodiscard]] std::size_t operator()(const std::string& value) const noexcept {
      return std::hash<std::string_view>{}(value);
    }
  };

  struct action_entry {
    std::vector<key_binding> bindings;
    action_state state;
  };

  using action_map = std::unordered_map<std::string, action_entry, action_hash, std::equal_to<>>;

  action_map actions_;
};

} // namespace platform
} // namespace cyberverse
