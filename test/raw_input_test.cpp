#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "cyberverse/platform/raw_input.hpp"

using namespace cyberverse::platform;

TEST_CASE("key state follows the frame protocol", "[raw_input][keyboard]") {
  raw_input_state state;

  CHECK_FALSE(state.is_key_down(key_id::key_a));

  state.key_down(key_id::key_a);
  CHECK(state.is_key_down(key_id::key_a));
  CHECK(state.is_key_pressed(key_id::key_a));
  CHECK_FALSE(state.is_key_released(key_id::key_a));

  state.begin_frame();
  CHECK(state.is_key_down(key_id::key_a));
  CHECK_FALSE(state.is_key_pressed(key_id::key_a));
  CHECK_FALSE(state.is_key_released(key_id::key_a));

  // Auto-repeat must not produce another pressed transition.
  state.key_down(key_id::key_a);
  CHECK_FALSE(state.is_key_pressed(key_id::key_a));

  state.key_up(key_id::key_a);
  CHECK_FALSE(state.is_key_down(key_id::key_a));
  CHECK(state.is_key_released(key_id::key_a));

  state.begin_frame();
  CHECK_FALSE(state.is_key_down(key_id::key_a));
  CHECK_FALSE(state.is_key_released(key_id::key_a));
}

TEST_CASE("invalid key ids are ignored", "[raw_input][keyboard]") {
  raw_input_state state;

  state.key_down(static_cast<key_id>(-1));
  state.key_down(static_cast<key_id>(10000));
  CHECK_FALSE(state.is_key_down(static_cast<key_id>(-1)));
  CHECK_FALSE(state.is_key_down(static_cast<key_id>(10000)));
}

TEST_CASE("mouse position, delta, and scroll accumulate per frame", "[raw_input][mouse]") {
  raw_input_state state;

  state.mouse_move(10.0f, 20.0f);
  CHECK(state.mouse_position().x == Catch::Approx(10.0f));
  CHECK(state.mouse_position().y == Catch::Approx(20.0f));
  CHECK(state.mouse_delta().x == Catch::Approx(0.0f));
  CHECK(state.mouse_delta().y == Catch::Approx(0.0f));

  state.begin_frame();
  state.mouse_move(15.0f, 25.0f);
  CHECK(state.mouse_position().x == Catch::Approx(15.0f));
  CHECK(state.mouse_position().y == Catch::Approx(25.0f));
  CHECK(state.mouse_delta().x == Catch::Approx(5.0f));
  CHECK(state.mouse_delta().y == Catch::Approx(5.0f));

  state.mouse_scroll(0.0f, 1.0f);
  state.mouse_scroll(1.5f, -3.0f);
  CHECK(state.scroll_delta().x == Catch::Approx(1.5f));
  CHECK(state.scroll_delta().y == Catch::Approx(-2.0f));

  state.begin_frame();
  CHECK(state.mouse_delta().x == Catch::Approx(0.0f));
  CHECK(state.mouse_delta().y == Catch::Approx(0.0f));
  CHECK(state.scroll_delta().x == Catch::Approx(0.0f));
  CHECK(state.scroll_delta().y == Catch::Approx(0.0f));
}

TEST_CASE("mouse buttons also update their key ids", "[raw_input][mouse]") {
  raw_input_state state;

  state.mouse_button_down(mouse_button::left);
  CHECK(state.is_mouse_button_down(mouse_button::left));
  CHECK(state.is_mouse_button_pressed(mouse_button::left));
  CHECK(state.is_key_down(key_id::mouse_left));
  CHECK(state.is_key_pressed(key_id::mouse_left));

  state.begin_frame();
  CHECK(state.is_mouse_button_down(mouse_button::left));
  CHECK_FALSE(state.is_mouse_button_pressed(mouse_button::left));

  state.mouse_button_up(mouse_button::left);
  CHECK_FALSE(state.is_mouse_button_down(mouse_button::left));
  CHECK(state.is_mouse_button_released(mouse_button::left));
  CHECK_FALSE(state.is_key_down(key_id::mouse_left));

  state.mouse_button_down(7);
  CHECK(state.is_mouse_button_down(7));
  CHECK(state.is_key_down(key_id::mouse_button8));
}

TEST_CASE("modifier state tracks transitions", "[raw_input][modifiers]") {
  raw_input_state state;

  CHECK(state.is_mod_down(mod_keys::none));
  CHECK_FALSE(state.is_mod_down(mod_keys::shift));

  state.set_mod_keys(mod_keys::shift | mod_keys::control);
  CHECK(state.is_mod_down(mod_keys::shift));
  CHECK(state.is_mod_down(mod_keys::control));
  CHECK(state.is_mod_down(mod_keys::shift | mod_keys::control));
  CHECK(state.is_mod_pressed(mod_keys::shift | mod_keys::control));
  CHECK_FALSE(state.is_mod_down(mod_keys::alt));

  state.begin_frame();
  CHECK(state.is_mod_down(mod_keys::shift));
  CHECK_FALSE(state.is_mod_pressed(mod_keys::shift));

  state.set_mod_keys(mod_keys::control);
  CHECK_FALSE(state.is_mod_down(mod_keys::shift));
  CHECK(state.is_mod_down(mod_keys::control));
  CHECK(state.is_mod_released(mod_keys::shift));
  CHECK_FALSE(state.is_mod_released(mod_keys::control));
}

TEST_CASE("modifier keys update modifier state from key events", "[raw_input][modifiers]") {
  raw_input_state state;

  state.key_down(key_id::key_left_shift);
  CHECK(state.is_mod_down(mod_keys::shift));

  state.key_up(key_id::key_left_shift);
  CHECK_FALSE(state.is_mod_down(mod_keys::shift));

  state.key_down(key_id::key_right_control);
  CHECK(state.is_mod_down(mod_keys::control));
}

TEST_CASE("text input is UTF-8 encoded", "[raw_input][text]") {
  raw_input_state state;

  state.add_text_input(U'A');
  state.add_text_input(U'\u00E9');
  state.add_text_input(U'\U0001F600');
  CHECK(state.text_input() == "A\xC3\xA9\xF0\x9F\x98\x80");

  state.add_text_input(static_cast<char32_t>(0xD800));
  CHECK(state.text_input() == "A\xC3\xA9\xF0\x9F\x98\x80");

  state.add_text_input(std::string_view{ "xyz" });
  CHECK(state.text_input() == "A\xC3\xA9\xF0\x9F\x98\x80xyz");

  state.begin_frame();
  CHECK(state.text_input().empty());
}

TEST_CASE("release_all releases held keys and buttons", "[raw_input][release_all]") {
  raw_input_state state;

  state.key_down(key_id::key_w);
  state.mouse_button_down(mouse_button::right);
  state.begin_frame();

  state.release_all();
  CHECK_FALSE(state.is_key_down(key_id::key_w));
  CHECK_FALSE(state.is_mouse_button_down(mouse_button::right));
  CHECK(state.is_key_released(key_id::key_w));
  CHECK(state.is_mouse_button_released(mouse_button::right));
}
