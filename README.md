# platform

Cyberverse Engine platform library. It owns the operating-system-facing parts of
the engine — windows, raw input, frame timing, key mapping, and system/process
information — and exposes the native window handle that the renderer needs.

See [docs/roadmap.md](docs/roadmap.md) for planned work and open design
questions.

## Backends

- **GLFW** (`backends/glfw`) — real windows through GLFW 3.4. Windows are created
  with `GLFW_NO_API`; the renderer creates its own device and swap chain. On
  macOS the backend installs a `CAMetalLayer` on the content view for MoltenVK
  and exposes the `NSWindow*`, `NSView*`, and layer through
  `native_window_handle`.
- **Headless** (`backends/headless`) — no native handle and a framebuffer scale
  of 1:1. Input and resize events are queued with `inject_*` methods and
  dispatched by `poll_events()`, which makes it suitable for deterministic tests
  and server builds. Always built.
- **SDL** — reserved enum value only. `CYBERVERSE_PLATFORM_BACKEND_SDL=ON` is a
  configure-time error and the backend is not implemented.

The GLFW backend can be disabled with `-DCYBERVERSE_PLATFORM_BACKEND_GLFW=OFF`;
`default_backend()` then returns headless.

## Frame loop

`window::poll_events()` starts a new input frame and then dispatches events, so
read input after polling. `timer::tick()` should run once at the start of each
frame.

```cpp
using namespace cyberverse::platform;

auto host = make_platform(default_backend());
auto window = host->create_window(window_config{ .title = "Example" });
auto frame_timer = host->create_timer();

key_mapping mapping;
mapping.bind("jump", key_id::key_space);
mapping.bind("fire", mouse_button::left);
mapping.bind("save", key_id::key_s, mod_keys::control);

while (!window->should_close()) {
  frame_timer->tick();
  window->poll_events();
  mapping.update(window->input_state());

  if (mapping.is_pressed("jump")) {
    // ...
  }

  const auto dt = static_cast<float>(frame_timer->delta_seconds());
  (void)dt;
  // render using window->width() x window->height() and window->native_handle()
}
```

- `key_id` values 0-7 are mouse buttons; keyboard values match GLFW key codes.
  Modifier bits match `GLFW_MOD_*`.
- `is_key_down()` and action `is_down()` persist across frames.
  `is_key_pressed()` / `is_key_released()` and action `is_pressed()` /
  `is_released()` are per-frame edges. Mouse and scroll deltas and text input are
  per-frame too.
- Key mappings require an exact modifier set; caps lock and num lock are ignored.
- `window::set_ime_caret()` reports the focused text caret rectangle (logical
  window content coordinates) for platform IME candidate placement. The default
  implementation is a no-op; headless windows store the latest value for tests.
- `window::get_ime_state()` returns the active platform IME preedit. GLFW on
  macOS captures marked text from the native `NSTextInputClient` bridge; headless
  windows expose `inject_ime_composition()` for deterministic tests.
- `window::clipboard_text()` / `set_clipboard_text()` expose the platform
  clipboard. GLFW uses the GLFW clipboard API; headless windows use an in-memory
  clipboard.
- `window_config.width` / `height` are the logical client size.
  `window::width()` / `height()` are framebuffer pixels, and
  `framebuffer_scale()` is pixels divided by logical size.
- Windows, timers, and the platform have a shared lifetime: created objects hold
  a release handler with a `shared_ptr` to the platform, so a window or timer may
  safely outlive the caller's platform reference.
- GLFW calls must happen on the main thread. Poll one window per application
  frame.

## System information

`platform::platform::query_system_info()`, `query_memory_info()`, and
`query_process_info()` provide best-effort cross-platform information without
tying it to a particular window backend:

```cpp
using namespace cyberverse::platform;

auto host = make_platform(backend_type::headless);

const system_info system = host->query_system_info();
const memory_info memory = host->query_memory_info();
const process_info process = host->query_process_info();
const std::optional<cpu_times> cpu = host->query_cpu_times();

// system.os_name, system.cpu_model, system.logical_core_count,
// system.total_memory_bytes, memory.available_memory_bytes,
// process.resident_bytes, process.virtual_bytes,
// cpu->total_ticks / cpu->idle_ticks (cumulative counters)
```

`query_cpu_times()` returns cumulative counters; callers compute utilization from
two snapshots. It is `std::nullopt` when the platform does not expose them.
Unsupported fields are empty or zero. The queries are thread-safe and do not
create windows or initialize a backend.

## Layout

```text
src/cyberverse/platform.hpp                    umbrella header
src/cyberverse/platform/
  backend.hpp                                  backend_type, backend_name
  platform.hpp / .cpp                          platform interface and factory
  window.hpp / .cpp                            window interface, config, native handle
  raw_input.hpp / .cpp                         key/mouse/text input state
  key_mapping.hpp / .cpp                       named actions over raw input
  timer.hpp / .cpp                             frame timer interface
  system_info.hpp / .cpp                       cross-platform OS/CPU/memory/process info
  detail/frame_timer.hpp / .cpp                steady-clock timer shared by backends
  detail/release_guard.hpp                     platform lifetime guard
  backends/glfw/                               GLFW platform/window/error/native helpers
  backends/glfw/macos/window.mm                CAMetalLayer + NSTextInputClient IME bridge
  backends/headless/                           headless platform/window with event injection
docs/roadmap.md                                development roadmap (Chinese)
```

## Build and test

Run from this directory. Build and run tests under `/tmp`, never in the repo.

```bash
cmake -S . -B /tmp/cyberverse-platform-test
cmake --build /tmp/cyberverse-platform-test
ctest --test-dir /tmp/cyberverse-platform-test --output-on-failure
```

The first configure fetches GLM 1.0.1, GLFW 3.4 (when enabled), Catch2 v3.5.2,
and Google Benchmark v1.9.5. Skip tests and benchmarks with
`-DCYBERVERSE_PLATFORM_BUILD_TESTS=OFF -DCYBERVERSE_PLATFORM_BUILD_BENCHMARKS=OFF`.

The real GLFW window smoke test is opt-in so routine runs do not open a window:

```bash
CYBERVERSE_PLATFORM_TEST_WINDOWS=1 /tmp/cyberverse-platform-test/test/cyberverse_platform_tests "[glfw]"
```

Presets: `cmake --preset debug` / `cmake --preset release`, then
`cmake --build --preset debug|release`. On macOS the GLFW backend is
Objective-C++ with ARC and links Cocoa and QuartzCore.

Format with the repository `.clang-format` (LLVM base, 2-space indent, 120
columns, attached braces, left-aligned pointers).
