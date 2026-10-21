/**
 * @file backend.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Backend
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

// The SDL backend is intentionally not implemented. The build system rejects
// CYBERVERSE_PLATFORM_BACKEND_SDL=ON before compilation; this guard also covers
// consumers that define the macro by hand.
#if defined(CYBERVERSE_PLATFORM_BACKEND_SDL) && CYBERVERSE_PLATFORM_BACKEND_SDL
#error "The SDL backend of cyberverse::platform is not implemented. Use GLFW or headless."
#endif

namespace cyberverse {
namespace platform {

/**
 * @brief Backend type
 *
 */
enum class backend_type { glfw, sdl, headless };

/**
 * @brief Human readable backend name
 *
 * @param type Backend type
 * @return const char*
 */
inline const char* backend_name(backend_type type) noexcept {
  switch (type) {
  case backend_type::glfw:
    return "glfw";
  case backend_type::sdl:
    return "sdl";
  case backend_type::headless:
    return "headless";
  }
  return "unknown";
}

} // namespace platform
} // namespace cyberverse
