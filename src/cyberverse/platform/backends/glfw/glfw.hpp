/**
 * @file glfw.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief GLFW error handling
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <ostream>
#include <string>
#include <system_error>

#include <GLFW/glfw3.h>

namespace cyberverse {
namespace platform {
namespace backends {
namespace glfw {

/**
 * @brief Error category
 *
 */
class error_category : public std::error_category {
public:
  [[nodiscard]] const char* name() const noexcept override { return "glfw error category"; }

  [[nodiscard]] std::string message(int ev) const override {
    switch (ev) {
    case GLFW_NOT_INITIALIZED:
      return "GLFW is not initialized";
    case GLFW_NO_CURRENT_CONTEXT:
      return "no current GLFW context";
    case GLFW_INVALID_ENUM:
      return "invalid GLFW enum";
    case GLFW_INVALID_VALUE:
      return "invalid GLFW value";
    case GLFW_OUT_OF_MEMORY:
      return "GLFW out of memory";
    case GLFW_API_UNAVAILABLE:
      return "requested GLFW API is unavailable";
    case GLFW_VERSION_UNAVAILABLE:
      return "requested GLFW version is unavailable";
    case GLFW_PLATFORM_ERROR:
      return "GLFW platform error";
    case GLFW_FORMAT_UNAVAILABLE:
      return "requested GLFW format is unavailable";
    case GLFW_NO_WINDOW_CONTEXT:
      return "GLFW window has no context";
    default:
      return "glfw error " + std::to_string(ev);
    }
  }
};

/**
 * @brief Get the glfw error category object
 *
 * @return const error_category&
 */
inline const error_category& get_error_category() noexcept {
  static const error_category instance;
  return instance;
}

/**
 * @brief Error definition
 *
 */
class error {
public:
  error(int code, const char* message) noexcept : code_(code), message_(message != nullptr ? message : "no message") {}

  ~error() = default;
  error(const error&) = default;
  error(error&&) = default;

  error& operator=(const error&) = default;
  error& operator=(error&&) = default;

  /**
   * @brief Convert to std::error_code
   *
   * @return std::error_code
   */
  [[nodiscard]] operator std::error_code() const noexcept { return error_code(); }

  /**
   * @brief Get error code
   *
   * @return int
   */
  [[nodiscard]] int code() const noexcept { return code_; }

  /**
   * @brief Get error message
   *
   * @return const char*
   */
  [[nodiscard]] const char* message() const noexcept { return message_.c_str(); }

  /**
   * @brief Get error code
   *
   * @return std::error_code
   */
  [[nodiscard]] std::error_code error_code() const noexcept { return std::error_code(code_, get_error_category()); }

private:
  int code_ = 0;
  std::string message_;
};

/**
 * @brief Get last glfw error code
 *
 * @return std::error_code
 */
inline std::error_code make_error_code() noexcept {
  return std::error_code(glfwGetError(nullptr), get_error_category());
}

/**
 * @brief Make glfw error
 *
 * @return error
 */
inline error make_error() noexcept {
  const char* message = nullptr;
  const auto code = glfwGetError(&message);
  return error(code, message);
}

/**
 * @brief Throw the last GLFW error as std::system_error
 *
 * Does nothing when GLFW reported no error.
 *
 * @param what Context description
 */
inline void throw_last_error(const char* what) {
  const auto code = glfwGetError(nullptr);
  if (code != GLFW_NO_ERROR) {
    throw std::system_error(std::error_code(code, get_error_category()), what);
  }
}

/**
 * @brief Standard string output of error
 *
 */
inline std::ostream& operator<<(std::ostream& os, const error& value) {
  os << '#' << value.code() << ':' << value.message();
  return os;
}

} // namespace glfw
} // namespace backends
} // namespace platform
} // namespace cyberverse
