# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository

`platform` is the standalone platform module of Cyberverse Engine: C++23, GLM, built as the static
library `cyberverse::platform::static_lib`. It provides windows, raw input, frame timing, key mapping,
and native window handles. It does **not** create a graphics device or swap chain; the renderer consumes
`native_window_handle` and owns rendering.

Dependencies: GLM 1.0.1 (always) and GLFW 3.4 (when the GLFW backend is enabled), both fetched by CMake.
There is no dependency on `geometry`, `transform`, `spatial-index`, or `vfs9`.

The public API lives in `src/cyberverse/platform/`; `src/cyberverse/platform.hpp` is the umbrella header.
Backends live under `src/cyberverse/platform/backends/<backend>/`.

## Backends

- `glfw` — real windows. Created with `GLFW_NO_API`. Initialization is process-wide and reference
  counted, so several platform objects can coexist; `glfwTerminate()` runs after the last one. On macOS
  `window.mm` installs a `CAMetalLayer` on the content view for MoltenVK and fills
  `native_window_handle` with `NSWindow*`, `NSView*`, and `CAMetalLayer*`. On Windows/Linux the native
  handle exposes `HWND`, `X11 Window + Display*`, or Wayland `wl_display* + wl_surface*`.
- `headless` — no native window. Framebuffer scale is 1:1. `inject_*` methods queue events that
  `poll_events()` dispatches, so input and resize behavior is deterministic in tests. Always built.
- `sdl` — **not implemented**. The enum value stays so callers can name it, `make_platform(sdl)` throws
  `std::invalid_argument`, `CYBERVERSE_PLATFORM_BACKEND_SDL=ON` is a configure-time `FATAL_ERROR`, and
  `backend.hpp` contains an `#error` guard if the macro is defined by hand. Do not add a silent fallback.

`CYBERVERSE_PLATFORM_BACKEND_GLFW` (default `ON`) controls the GLFW backend. `default_backend()` returns
GLFW when it was built, otherwise headless. `supported_backends()` reflects the build configuration.

## Build and test

Run from this directory. Build and run tests under `/tmp`, never inside the repository.

```bash
cmake -S . -B /tmp/cyberverse-platform-test
cmake --build /tmp/cyberverse-platform-test
ctest --test-dir /tmp/cyberverse-platform-test --output-on-failure
```

- Headless-only build: add `-DCYBERVERSE_PLATFORM_BACKEND_GLFW=OFF`.
- Skip dependencies: `-DCYBERVERSE_PLATFORM_BUILD_TESTS=OFF -DCYBERVERSE_PLATFORM_BUILD_BENCHMARKS=OFF`.
- Presets: `cmake --preset debug` / `cmake --preset release`, then
  `cmake --build --preset debug|release`; outputs go to `build/` and `build-release/`. Tests still must
  be built and run under `/tmp`.
- `test/` builds `cyberverse_platform_tests` (Catch2, discovered by CTest); `benchmark/` builds
  `cyberverse_platform_benchmarks` (Google Benchmark).
- Real GLFW window creation is opt-in:
  `CYBERVERSE_PLATFORM_TEST_WINDOWS=1 /tmp/cyberverse-platform-test/test/cyberverse_platform_tests "[glfw]"`.
  Without the variable the smoke test succeeds without opening a window.
- On macOS the GLFW backend is Objective-C++ (`src/.../glfw/macos/window.mm`, ARC enabled) and needs
  `OBJCXX` enabled plus Cocoa/QuartzCore. CMake enables `OBJCXX` only when `APPLE AND
  CYBERVERSE_PLATFORM_BACKEND_GLFW`.
- Format with `.clang-format` (LLVM base, 2-space indent, 120 columns, attached braces, left-aligned
  pointers): `clang-format -i <files>` (on macOS, `xcrun clang-format` works when `clang-format` is not
  on `PATH`).

## Frame and input semantics

- Call `window::poll_events()` once per application frame before reading input. It starts a new input
  frame (`raw_input_state::begin_frame()`) and then dispatches platform events.
- Call `timer::tick()` once at the start of each frame. The first tick starts the steady clock and
  reports delta 0; `total_seconds()` is measured from the first tick. `frame_count()` counts ticks.
- Persistent state: key/button down state and mouse position.
- Per-frame state: pressed/released edges, modifier edges, mouse delta, scroll delta, and text input.
  They are cleared by the next `begin_frame()`.
- Key auto-repeat does not create repeated pressed edges; repeated `key_down()` calls while the key is
  down are ignored.
- Modifier bits come from `set_mod_keys()` (GLFW passes its `mods` argument). Modifier key press/release
  events also update shift/control/alt/super. Caps lock and num lock are toggle states supplied by the
  backend and are not derived from key events.
- `release_all()` (used on focus loss) releases every key and mouse button and reports release edges.
- Mouse buttons update both `mouse_buttons_*` and the matching `key_id::mouse_button*` key state, so key
  mappings can bind mouse buttons.
- Headless windows queue events and dispatch them in order during `poll_events()`, after
  `begin_frame()`. This ordering is what makes `inject_*` + `poll_events()` produce the same edges as
  native events.

## Windows and timing

- `window_config.width` / `height` are the logical client size. `window::width()` / `height()` are the
  framebuffer size in pixels; `framebuffer_scale()` is framebuffer pixels divided by logical size.
  Headless windows are always 1:1.
- The resize callback receives the new framebuffer size in pixels.
- `window_config.vsync` is a hint for the renderer's swap chain; the platform does not create one.
- `key_id` values 0-7 are mouse buttons. Keyboard values and `mod_keys` bits match GLFW key codes and
  `GLFW_MOD_*` constants; keep that alignment if another backend is added.
- GLFW windows must be created, polled, and destroyed on the main thread. Poll one window per frame in
  a multi-window application.

## Key mapping

`key_mapping` maps physical keys and mouse buttons to named actions. Call `update(state)` once per frame
after polling, then query `is_down` / `is_pressed` / `is_released` / `state`.

- Multiple bindings may map to one action; the action is down while any active binding is down.
- A binding requires an exact modifier set: control+S does not trigger while shift or alt is also held.
  Caps lock and num lock are ignored for matching.
- `pressed` means at least one active binding went down this frame. `released` is the action's
  down-to-up edge, so releasing one of several held bindings does not release the action.
- Duplicate bindings are ignored. `unbind` removes one binding; `unbind_all` removes the action.
- `update` does not allocate for registered actions and is declared `noexcept`.

## Lifetime

`platform` inherits `std::enable_shared_from_this`. `create_timer()` and `create_window()` capture a
`shared_ptr` to the platform in a release handler (`detail::release_guard`), which the child invokes on
destruction. A timer or window can therefore outlive the caller's platform reference, and the platform
is destroyed after its last child. Preserve this contract when adding backends or new child objects.

Windows and timers are non-copyable and non-movable. The platform factory returns `std::shared_ptr`.

## Style

- Public types and functions are `lower_snake_case`; private members end with `_`.
- Write the return type before the function name (`bool is_down(...)`), not as a trailing return type (`auto is_down(...) -> bool`). Use a trailing return only when the type cannot be spelled otherwise.
- Headers use `#pragma once`, live under `src/cyberverse/platform/`, and are included as
  `"cyberverse/platform/<name>.hpp"`. Internal helpers live in `detail/`; backend-specific public
  surfaces live under `backends/<backend>/`.
- Add new public headers and sources to `HEADER_FILES` / `SOURCE_FILES` in `src/CMakeLists.txt`, and
  include public umbrella headers from `src/cyberverse/platform.hpp` when appropriate.
- Keep the Doxygen `@file` header at the top of new files.
- Platform-facing mutators on `raw_input_state` are public because backends live in other namespaces;
  applications should read the state, not mutate it directly.

## Git

- Existing repo remotes (`origin`) use the local SSH alias `github-simon2077s.com`; don't rewrite them.
  This alias is not available on other machines or CI.
- Build-time fetches must use the public host (`https://github.com/...`), never the alias.
- Commit directly on `main`, using Conventional Commits (`feat:`, `fix:`, `docs:`, `chore:`).

`AGENTS.md` is a symlink to this file; edit only `CLAUDE.md`.
