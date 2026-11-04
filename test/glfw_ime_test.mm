#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <memory>
#include <system_error>

#import <Cocoa/Cocoa.h>

#include "cyberverse/platform.hpp"

using namespace cyberverse::platform;

TEST_CASE("macOS GLFW captures IME composition and caret", "[glfw][ime]") {
#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW && defined(__APPLE__)
  if (std::getenv("CYBERVERSE_PLATFORM_TEST_WINDOWS") == nullptr) {
    SUCCEED("set CYBERVERSE_PLATFORM_TEST_WINDOWS=1 to create a real GLFW window");
    return;
  }

  std::shared_ptr<platform> host;
  try {
    host = make_platform(backend_type::glfw);
  } catch (const std::system_error& error) {
    WARN("GLFW is unavailable in this environment: " << error.what());
    SUCCEED("GLFW initialization is environment dependent");
    return;
  }

  std::shared_ptr<window> window;
  try {
    window_config config;
    config.width = 320;
    config.height = 240;
    config.title = "cyberverse IME test";
    window = host->create_window(config);
  } catch (const std::system_error& error) {
    WARN("cannot create a GLFW window in this environment: " << error.what());
    SUCCEED("GLFW window creation is environment dependent");
    return;
  }
  REQUIRE(window != nullptr);

  NSView* view = (__bridge NSView*)window->native_handle().view;
  REQUIRE(view != nil);
  id<NSTextInputClient> input = (id<NSTextInputClient>)view;

  NSString* text = @"\xE4\xBD\xA0\xE5\xA5\xBD";
  [input setMarkedText:text selectedRange:NSMakeRange([text length], 0) replacementRange:NSMakeRange(NSNotFound, 0)];
  ime_state state = window->get_ime_state();
  REQUIRE(state.active);
  REQUIRE(state.text == "\xE4\xBD\xA0\xE5\xA5\xBD");
  REQUIRE(state.cursor == 6);

  window->set_ime_caret(ime_caret{ 12.0f, 20.0f, 18.0f, true });
  NSRect candidate = [input firstRectForCharacterRange:NSMakeRange(0, 0) actualRange:nullptr];
  CHECK(candidate.size.height > 0.0);

  [input unmarkText];
  REQUIRE_FALSE(window->get_ime_state().active);

  window.reset();
  host.reset();
#else
  SUCCEED("the GLFW backend or Cocoa is not available");
#endif
}
