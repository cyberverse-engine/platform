#include <catch2/catch_test_macros.hpp>

#include "cyberverse/platform/key_mapping.hpp"

using namespace cyberverse::platform;

TEST_CASE("key mapping reports press, hold, and release edges", "[key_mapping]") {
  raw_input_state input;
  key_mapping mapping;
  mapping.bind("jump", key_id::key_space);

  mapping.update(input);
  CHECK_FALSE(mapping.is_down("jump"));
  CHECK_FALSE(mapping.is_pressed("jump"));
  CHECK_FALSE(mapping.is_released("jump"));

  input.key_down(key_id::key_space);
  mapping.update(input);
  CHECK(mapping.is_down("jump"));
  CHECK(mapping.is_pressed("jump"));
  CHECK_FALSE(mapping.is_released("jump"));

  input.begin_frame();
  mapping.update(input);
  CHECK(mapping.is_down("jump"));
  CHECK_FALSE(mapping.is_pressed("jump"));

  input.key_up(key_id::key_space);
  mapping.update(input);
  CHECK_FALSE(mapping.is_down("jump"));
  CHECK_FALSE(mapping.is_pressed("jump"));
  CHECK(mapping.is_released("jump"));

  input.begin_frame();
  mapping.update(input);
  CHECK_FALSE(mapping.is_released("jump"));
}

TEST_CASE("an action can have multiple bindings", "[key_mapping]") {
  raw_input_state input;
  key_mapping mapping;
  mapping.bind("forward", key_id::key_w);
  mapping.bind("forward", key_id::key_up);
  CHECK(mapping.bindings("forward").size() == 2);

  input.key_down(key_id::key_up);
  mapping.update(input);
  CHECK(mapping.is_down("forward"));
  CHECK(mapping.is_pressed("forward"));

  input.key_up(key_id::key_up);
  input.key_down(key_id::key_w);
  mapping.update(input);
  CHECK(mapping.is_down("forward"));
  CHECK_FALSE(mapping.is_released("forward"));
}

TEST_CASE("bindings require an exact modifier set", "[key_mapping][modifiers]") {
  raw_input_state input;
  key_mapping mapping;
  mapping.bind("save", key_id::key_s, mod_keys::control);

  input.set_mod_keys(mod_keys::control);
  input.key_down(key_id::key_s);
  mapping.update(input);
  CHECK(mapping.is_down("save"));

  input.set_mod_keys(mod_keys::control | mod_keys::shift);
  mapping.update(input);
  CHECK_FALSE(mapping.is_down("save"));
  CHECK(mapping.is_released("save"));

  input.set_mod_keys(mod_keys::shift);
  mapping.update(input);
  CHECK_FALSE(mapping.is_down("save"));

  input.set_mod_keys(mod_keys::control);
  mapping.update(input);
  CHECK(mapping.is_down("save"));
}

TEST_CASE("caps lock and num lock do not affect modifier matching", "[key_mapping][modifiers]") {
  raw_input_state input;
  key_mapping mapping;
  mapping.bind("jump", key_id::key_space);

  input.set_mod_keys(mod_keys::caps_lock | mod_keys::num_lock);
  input.key_down(key_id::key_space);
  mapping.update(input);
  CHECK(mapping.is_down("jump"));
}

TEST_CASE("mouse buttons can be bound", "[key_mapping][mouse]") {
  raw_input_state input;
  key_mapping mapping;
  mapping.bind("fire", mouse_button::left);
  mapping.bind("aim", mouse_button::right, mod_keys::shift);

  input.mouse_button_down(mouse_button::left);
  mapping.update(input);
  CHECK(mapping.is_down("fire"));
  CHECK_FALSE(mapping.is_down("aim"));

  input.begin_frame();
  input.set_mod_keys(mod_keys::shift);
  input.mouse_button_down(mouse_button::right);
  mapping.update(input);
  CHECK(mapping.is_down("aim"));
}

TEST_CASE("duplicate bindings are ignored and can be unbound", "[key_mapping]") {
  key_mapping mapping;
  mapping.bind("jump", key_id::key_space);
  mapping.bind("jump", key_id::key_space);
  CHECK(mapping.bindings("jump").size() == 1);

  CHECK_FALSE(mapping.unbind("jump", key_id::key_a));
  CHECK(mapping.unbind("jump", key_id::key_space));
  CHECK(mapping.bindings("jump").empty());
  CHECK(mapping.unbind_all("jump"));

  mapping.bind("other", key_id::key_space);
  CHECK(mapping.action_count() == 1);
  mapping.clear();
  CHECK(mapping.action_count() == 0);
  CHECK_FALSE(mapping.is_down("jump"));
}

TEST_CASE("unknown actions report an empty state", "[key_mapping]") {
  const key_mapping mapping;
  const auto state = mapping.state("missing");
  CHECK_FALSE(state.down);
  CHECK_FALSE(state.pressed);
  CHECK_FALSE(state.released);
  CHECK(mapping.bindings("missing").empty());
}
