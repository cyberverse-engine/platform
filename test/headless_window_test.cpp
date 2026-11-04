#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "cyberverse/platform.hpp"
#include "cyberverse/platform/backends/headless/window.hpp"

using namespace cyberverse::platform;

namespace {

std::shared_ptr<backends::headless::window> make_headless_window(const window_config& config = window_config{}) {
  static auto host = make_platform(backend_type::headless);
  return std::dynamic_pointer_cast<backends::headless::window>(host->create_window(config));
}

} // namespace

TEST_CASE("headless window reports its configuration", "[headless][window]") {
  window_config config;
  config.width = 640;
  config.height = 480;
  config.title = "headless test";

  auto window = make_headless_window(config);
  REQUIRE(window != nullptr);
  CHECK(window->backend_type() == backend_type::headless);
  CHECK(window->width() == 640);
  CHECK(window->height() == 480);
  CHECK(window->framebuffer_scale().x == Catch::Approx(1.0f));
  CHECK(window->framebuffer_scale().y == Catch::Approx(1.0f));
  CHECK(window->should_close() == false);
  CHECK(window->native_handle().backend == backend_type::headless);
  CHECK(window->native_handle().window == nullptr);
  CHECK(window->native_handle().display == nullptr);
  CHECK(window->native_handle().surface == nullptr);
  CHECK(window->get_cursor_mode() == cursor_mode::normal);
}

TEST_CASE("headless window dispatches injected input", "[headless][window]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  window->inject_key_down(key_id::key_space);
  window->inject_mouse_move(5.0f, 6.0f);
  window->inject_mouse_scroll(0.0f, 1.0f);
  window->inject_text(U'x');

  window->poll_events();
  CHECK(window->input_state().is_key_down(key_id::key_space));
  CHECK(window->input_state().is_key_pressed(key_id::key_space));
  CHECK(window->input_state().mouse_position().x == Catch::Approx(5.0f));
  CHECK(window->input_state().scroll_delta().y == Catch::Approx(1.0f));
  CHECK(window->input_state().text_input() == "x");

  window->poll_events();
  CHECK(window->input_state().is_key_down(key_id::key_space));
  CHECK_FALSE(window->input_state().is_key_pressed(key_id::key_space));
  CHECK(window->input_state().scroll_delta().y == Catch::Approx(0.0f));
  CHECK(window->input_state().text_input().empty());

  window->inject_key_up(key_id::key_space);
  window->poll_events();
  CHECK_FALSE(window->input_state().is_key_down(key_id::key_space));
  CHECK(window->input_state().is_key_released(key_id::key_space));
}

TEST_CASE("headless window dispatches resize events", "[headless][window]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  int resized_width = 0;
  int resized_height = 0;
  window->set_resize_callback([&](int width, int height) {
    resized_width = width;
    resized_height = height;
  });

  window->inject_resize(800, 600);
  CHECK(window->width() == 1280);
  CHECK(window->height() == 720);

  window->poll_events();
  CHECK(window->width() == 800);
  CHECK(window->height() == 600);
  CHECK(resized_width == 800);
  CHECK(resized_height == 600);
}

TEST_CASE("headless window cursor mode and close requests", "[headless][window]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  window->set_cursor_mode(cursor_mode::disabled);
  CHECK(window->get_cursor_mode() == cursor_mode::disabled);

  window->request_close();
  CHECK(window->should_close());
}

TEST_CASE("headless focus loss releases held input", "[headless][window]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  window->inject_key_down(key_id::key_w);
  window->inject_mouse_button_down(mouse_button::left);
  window->poll_events();
  CHECK(window->input_state().is_key_down(key_id::key_w));

  window->inject_focus_lost();
  window->poll_events();
  CHECK_FALSE(window->input_state().is_key_down(key_id::key_w));
  CHECK_FALSE(window->input_state().is_mouse_button_down(mouse_button::left));
  CHECK(window->input_state().is_key_released(key_id::key_w));
}

TEST_CASE("headless window records the IME caret", "[headless][window][ime]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  window->set_ime_caret(ime_caret{ 12.0f, 34.0f, 22.0f, true });
  const ime_caret caret = window->last_ime_caret();
  CHECK(caret.x == Catch::Approx(12.0f));
  CHECK(caret.y == Catch::Approx(34.0f));
  CHECK(caret.height == Catch::Approx(22.0f));
  CHECK(caret.visible);

  window->set_ime_caret(ime_caret{});
  CHECK_FALSE(window->last_ime_caret().visible);
}

TEST_CASE("headless window stores IME composition", "[headless][window][ime]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  window->inject_ime_composition("\xE4\xBD\xA0\xE5\xA5\xBD", 3);
  ime_state state = window->get_ime_state();
  CHECK(state.active);
  CHECK(state.text == "\xE4\xBD\xA0\xE5\xA5\xBD");
  CHECK(state.cursor == 3);

  window->inject_focus_lost();
  window->poll_events();
  CHECK_FALSE(window->get_ime_state().active);
  CHECK(window->get_ime_state().text.empty());

  window->inject_ime_composition("a", 99);
  CHECK(window->get_ime_state().cursor == 1);
  window->clear_ime_composition();
  CHECK_FALSE(window->get_ime_state().active);
}

TEST_CASE("headless window stores clipboard text", "[headless][window][clipboard]") {
  auto window = make_headless_window();
  REQUIRE(window != nullptr);

  CHECK(window->clipboard_text().empty());
  window->set_clipboard_text("hello");
  CHECK(window->clipboard_text() == "hello");
  window->set_clipboard_text("");
  CHECK(window->clipboard_text().empty());
}

TEST_CASE("windows keep their platform alive", "[headless][window][lifetime]") {
  auto host = make_platform(backend_type::headless);
  auto window = host->create_window(window_config{});

  host.reset();
  REQUIRE(window != nullptr);
  CHECK(window->width() == 1280);
  window->poll_events();
  CHECK_NOTHROW(window->request_close());
}
