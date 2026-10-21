/**
 * @file platform.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Platform factory and backend registry
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/platform.hpp"

#include <array>
#include <stdexcept>

#include "cyberverse/platform/backends/headless/platform.hpp"

#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW
#include "cyberverse/platform/backends/glfw/platform.hpp"
#endif

namespace cyberverse::platform {

std::shared_ptr<platform> make_platform(backend_type type) {
  switch (type) {
  case backend_type::glfw:
#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW
    return std::make_shared<backends::glfw::platform>();
#else
    throw std::invalid_argument(
        "cyberverse::platform: the GLFW backend was not built; enable CYBERVERSE_PLATFORM_BACKEND_GLFW");
#endif
  case backend_type::headless:
    return std::make_shared<backends::headless::platform>();
  case backend_type::sdl:
    throw std::invalid_argument("cyberverse::platform: the SDL backend is not implemented; use GLFW or headless");
  }

  throw std::invalid_argument("cyberverse::platform: unknown backend type");
}

backend_type default_backend() noexcept {
#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW
  return backend_type::glfw;
#else
  return backend_type::headless;
#endif
}

std::span<const backend_type> supported_backends() noexcept {
#if defined(CYBERVERSE_PLATFORM_BACKEND_GLFW) && CYBERVERSE_PLATFORM_BACKEND_GLFW
  static constexpr std::array<backend_type, 2> backends{ backend_type::glfw, backend_type::headless };
#else
  static constexpr std::array<backend_type, 1> backends{ backend_type::headless };
#endif
  return backends;
}

} // namespace cyberverse::platform
