cmake_minimum_required(VERSION 4.0.0)
project(cyberverse_platform CXX)

option(CYBERVERSE_PLATFORM_BUILD_TESTS "Build unit test cases" ON)
option(CYBERVERSE_PLATFORM_BUILD_BENCHMARKS "Build benchmarks" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 23)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

option(CYBERVERSE_PLATFORM_BACKEND_GLFW "Build GLFW backend" ON)
option(CYBERVERSE_PLATFORM_BACKEND_SDL "Build SDL backend (not implemented)" OFF)

if(CYBERVERSE_PLATFORM_BACKEND_SDL)
  message(FATAL_ERROR "The SDL backend of cyberverse::platform is not implemented. Use GLFW or headless.")
endif()

# The macOS backend installs a CAMetalLayer on the GLFW content view for MoltenVK.
if(APPLE AND CYBERVERSE_PLATFORM_BACKEND_GLFW)
  enable_language(OBJCXX)
endif()

add_subdirectory(src)

if(CYBERVERSE_PLATFORM_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(CYBERVERSE_PLATFORM_BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()
