/**
 * @file raw_input.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Raw input implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/raw_input.hpp"

#include <bit>
#include <cstdint>

namespace cyberverse::platform {

namespace {

constexpr std::size_t to_index(key_id key) noexcept { return static_cast<std::size_t>(static_cast<int>(key)); }

constexpr std::size_t to_mod_index(mod_keys mod) noexcept {
  return static_cast<std::size_t>(std::countr_zero(static_cast<unsigned int>(mod)));
}

mod_keys modifier_flag(key_id key) noexcept {
  switch (key) {
  case key_id::key_left_shift:
  case key_id::key_right_shift:
    return mod_keys::shift;
  case key_id::key_left_control:
  case key_id::key_right_control:
    return mod_keys::control;
  case key_id::key_left_alt:
  case key_id::key_right_alt:
    return mod_keys::alt;
  case key_id::key_left_super:
  case key_id::key_right_super:
    return mod_keys::super;
  default:
    return mod_keys::none;
  }
}

} // namespace

void raw_input_state::begin_frame() noexcept {
  keys_pressed_this_frame_.reset();
  keys_released_this_frame_.reset();

  mod_keys_pressed_this_frame_.reset();
  mod_keys_released_this_frame_.reset();

  mouse_buttons_pressed_this_frame_.reset();
  mouse_buttons_released_this_frame_.reset();

  mouse_delta_ = glm::vec2{ 0.0f };
  scroll_delta_ = glm::vec2{ 0.0f };
  text_input_.clear();
}

void raw_input_state::release_all() noexcept {
  for (int index = 0; index < static_cast<int>(key_capacity); ++index) {
    if (keys_down_.test(static_cast<std::size_t>(index))) {
      key_up(static_cast<key_id>(index));
    }
  }
  for (int index = 0; index < static_cast<int>(mouse_capacity); ++index) {
    if (mouse_buttons_down_.test(static_cast<std::size_t>(index))) {
      mouse_button_up(index);
    }
  }
}

bool raw_input_state::is_key_down(key_id key) const noexcept {
  return valid_key(key) && keys_down_.test(to_index(key));
}

bool raw_input_state::is_key_pressed(key_id key) const noexcept {
  return valid_key(key) && keys_pressed_this_frame_.test(to_index(key));
}

bool raw_input_state::is_key_released(key_id key) const noexcept {
  return valid_key(key) && keys_released_this_frame_.test(to_index(key));
}

bool raw_input_state::is_mod_down(mod_keys mods) const noexcept {
  const auto requested = static_cast<int>(mods);
  const auto current = static_cast<int>(mods_down());
  return (requested & ~current) == 0;
}

bool raw_input_state::is_mod_pressed(mod_keys mods) const noexcept {
  const auto requested = static_cast<int>(mods);
  int pressed = 0;
  for (std::size_t bit = 0; bit < mod_capacity; ++bit) {
    if (mod_keys_pressed_this_frame_.test(bit)) {
      pressed |= 1 << bit;
    }
  }
  return (requested & ~pressed) == 0;
}

bool raw_input_state::is_mod_released(mod_keys mods) const noexcept {
  const auto requested = static_cast<int>(mods);
  int released = 0;
  for (std::size_t bit = 0; bit < mod_capacity; ++bit) {
    if (mod_keys_released_this_frame_.test(bit)) {
      released |= 1 << bit;
    }
  }
  return (requested & ~released) == 0;
}

mod_keys raw_input_state::mods_down() const noexcept {
  int value = 0;
  for (std::size_t bit = 0; bit < mod_capacity; ++bit) {
    if (mod_keys_down_.test(bit)) {
      value |= 1 << bit;
    }
  }
  return static_cast<mod_keys>(value);
}

bool raw_input_state::is_mouse_button_down(mouse_button button) const noexcept {
  return is_mouse_button_down(static_cast<int>(button));
}

bool raw_input_state::is_mouse_button_down(int button) const noexcept {
  return valid_mouse_button(button) && mouse_buttons_down_.test(static_cast<std::size_t>(button));
}

bool raw_input_state::is_mouse_button_pressed(mouse_button button) const noexcept {
  return is_mouse_button_pressed(static_cast<int>(button));
}

bool raw_input_state::is_mouse_button_pressed(int button) const noexcept {
  return valid_mouse_button(button) && mouse_buttons_pressed_this_frame_.test(static_cast<std::size_t>(button));
}

bool raw_input_state::is_mouse_button_released(mouse_button button) const noexcept {
  return is_mouse_button_released(static_cast<int>(button));
}

bool raw_input_state::is_mouse_button_released(int button) const noexcept {
  return valid_mouse_button(button) && mouse_buttons_released_this_frame_.test(static_cast<std::size_t>(button));
}

glm::vec2 raw_input_state::mouse_position() const noexcept { return mouse_position_; }

glm::vec2 raw_input_state::mouse_delta() const noexcept { return mouse_delta_; }

glm::vec2 raw_input_state::scroll_delta() const noexcept { return scroll_delta_; }

std::string_view raw_input_state::text_input() const noexcept { return text_input_; }

void raw_input_state::key_down(key_id key) noexcept { set_key(key, true); }

void raw_input_state::key_up(key_id key) noexcept { set_key(key, false); }

void raw_input_state::set_mod_keys(mod_keys mods) noexcept {
  const auto value = static_cast<int>(mods);
  for (std::size_t bit = 0; bit < mod_capacity; ++bit) {
    set_mod_bit(static_cast<mod_keys>(1 << bit), (value & (1 << bit)) != 0);
  }
}

void raw_input_state::mouse_button_down(mouse_button button) noexcept { mouse_button_down(static_cast<int>(button)); }

void raw_input_state::mouse_button_down(int button) noexcept { set_mouse_button(button, true); }

void raw_input_state::mouse_button_up(mouse_button button) noexcept { mouse_button_up(static_cast<int>(button)); }

void raw_input_state::mouse_button_up(int button) noexcept { set_mouse_button(button, false); }

void raw_input_state::mouse_move(float x, float y) noexcept {
  const glm::vec2 position{ x, y };
  if (has_mouse_position_) {
    mouse_delta_ += position - mouse_position_;
  } else {
    has_mouse_position_ = true;
  }
  mouse_position_ = position;
}

void raw_input_state::mouse_scroll(float x, float y) noexcept { scroll_delta_ += glm::vec2{ x, y }; }

void raw_input_state::add_text_input(char32_t codepoint) {
  const auto value = static_cast<std::uint32_t>(codepoint);
  if (value > 0x10FFFFu || (value >= 0xD800u && value <= 0xDFFFu)) {
    return;
  }

  char buffer[4]{};
  std::size_t length = 0;
  if (value <= 0x7Fu) {
    buffer[0] = static_cast<char>(value);
    length = 1;
  } else if (value <= 0x7FFu) {
    buffer[0] = static_cast<char>(0xC0u | (value >> 6));
    buffer[1] = static_cast<char>(0x80u | (value & 0x3Fu));
    length = 2;
  } else if (value <= 0xFFFFu) {
    buffer[0] = static_cast<char>(0xE0u | (value >> 12));
    buffer[1] = static_cast<char>(0x80u | ((value >> 6) & 0x3Fu));
    buffer[2] = static_cast<char>(0x80u | (value & 0x3Fu));
    length = 3;
  } else {
    buffer[0] = static_cast<char>(0xF0u | (value >> 18));
    buffer[1] = static_cast<char>(0x80u | ((value >> 12) & 0x3Fu));
    buffer[2] = static_cast<char>(0x80u | ((value >> 6) & 0x3Fu));
    buffer[3] = static_cast<char>(0x80u | (value & 0x3Fu));
    length = 4;
  }
  text_input_.append(buffer, length);
}

void raw_input_state::add_text_input(std::string_view text) { text_input_.append(text); }

bool raw_input_state::valid_key(key_id key) noexcept {
  const auto value = static_cast<int>(key);
  return value >= 0 && value < static_cast<int>(key_capacity);
}

bool raw_input_state::valid_mouse_button(int button) noexcept {
  return button >= 0 && button < static_cast<int>(mouse_capacity);
}

void raw_input_state::set_key(key_id key, bool down) noexcept {
  if (!valid_key(key)) {
    return;
  }

  const auto index = to_index(key);
  if (keys_down_.test(index) == down) {
    return;
  }

  keys_down_.set(index, down);
  if (down) {
    keys_pressed_this_frame_.set(index);
  } else {
    keys_released_this_frame_.set(index);
  }

  update_modifier_from_key(key, down);
}

void raw_input_state::set_mouse_button(int button, bool down) noexcept {
  if (!valid_mouse_button(button)) {
    return;
  }

  const auto index = static_cast<std::size_t>(button);
  if (mouse_buttons_down_.test(index) != down) {
    mouse_buttons_down_.set(index, down);
    if (down) {
      mouse_buttons_pressed_this_frame_.set(index);
    } else {
      mouse_buttons_released_this_frame_.set(index);
    }
  }

  set_key(static_cast<key_id>(button), down);
}

void raw_input_state::set_mod_bit(mod_keys bit, bool down) noexcept {
  if (bit == mod_keys::none) {
    return;
  }

  const auto index = to_mod_index(bit);
  if (index >= mod_capacity || mod_keys_down_.test(index) == down) {
    return;
  }

  mod_keys_down_.set(index, down);
  if (down) {
    mod_keys_pressed_this_frame_.set(index);
  } else {
    mod_keys_released_this_frame_.set(index);
  }
}

void raw_input_state::update_modifier_from_key(key_id key, bool down) noexcept {
  const auto flag = modifier_flag(key);
  if (flag != mod_keys::none) {
    set_mod_bit(flag, down);
  }
}

} // namespace cyberverse::platform
