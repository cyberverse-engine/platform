/**
 * @file key_mapping.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Key map implementation
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/key_mapping.hpp"

#include <algorithm>

namespace cyberverse::platform {

namespace {

constexpr mod_keys modifier_mask() noexcept {
  return mod_keys::shift | mod_keys::control | mod_keys::alt | mod_keys::super;
}

constexpr bool modifiers_match(mod_keys current, mod_keys required) noexcept {
  const auto mask = modifier_mask();
  return (current & mask) == (required & mask);
}

} // namespace

void key_mapping::bind(std::string_view action, key_id key, mod_keys modifiers) {
  bind(action, key_binding{ key, modifiers });
}

void key_mapping::bind(std::string_view action, mouse_button button, mod_keys modifiers) {
  bind(action, key_binding{ static_cast<key_id>(static_cast<int>(button)), modifiers });
}

void key_mapping::bind(std::string_view action, key_binding binding) {
  binding.modifiers &= modifier_mask();
  auto& entry = actions_[std::string(action)];
  if (std::find(entry.bindings.begin(), entry.bindings.end(), binding) == entry.bindings.end()) {
    entry.bindings.push_back(binding);
  }
}

bool key_mapping::unbind(std::string_view action, key_id key, mod_keys modifiers) {
  const auto action_it = actions_.find(action);
  if (action_it == actions_.end()) {
    return false;
  }

  const key_binding target{ key, modifiers & modifier_mask() };
  auto& bindings = action_it->second.bindings;
  const auto binding_it = std::find(bindings.begin(), bindings.end(), target);
  if (binding_it == bindings.end()) {
    return false;
  }

  bindings.erase(binding_it);
  return true;
}

bool key_mapping::unbind_all(std::string_view action) {
  const auto action_it = actions_.find(action);
  if (action_it == actions_.end()) {
    return false;
  }
  actions_.erase(action_it);
  return true;
}

void key_mapping::clear() noexcept { actions_.clear(); }

void key_mapping::update(const raw_input_state& state) noexcept {
  const auto mods = state.mods_down();
  for (auto& [action, entry] : actions_) {
    (void)action;
    bool down = false;
    bool pressed = false;
    for (const auto& binding : entry.bindings) {
      if (!modifiers_match(mods, binding.modifiers)) {
        continue;
      }
      down = down || state.is_key_down(binding.key);
      pressed = pressed || state.is_key_pressed(binding.key);
    }

    entry.state.released = entry.state.down && !down;
    entry.state.down = down;
    entry.state.pressed = pressed;
  }
}

bool key_mapping::is_down(std::string_view action) const noexcept { return state(action).down; }

bool key_mapping::is_pressed(std::string_view action) const noexcept { return state(action).pressed; }

bool key_mapping::is_released(std::string_view action) const noexcept { return state(action).released; }

action_state key_mapping::state(std::string_view action) const noexcept {
  const auto it = actions_.find(action);
  return it == actions_.end() ? action_state{} : it->second.state;
}

std::span<const key_binding> key_mapping::bindings(std::string_view action) const noexcept {
  const auto it = actions_.find(action);
  return it == actions_.end() ? std::span<const key_binding>{} : std::span<const key_binding>{ it->second.bindings };
}

std::size_t key_mapping::action_count() const noexcept { return actions_.size(); }

} // namespace cyberverse::platform
