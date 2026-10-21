cmake_minimum_required(VERSION 4.0.0)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(Catch2)

add_executable(
  cyberverse_platform_tests
  raw_input_test.cpp
  key_mapping_test.cpp
  timer_test.cpp
  platform_test.cpp
  headless_window_test.cpp
  glfw_window_test.cpp
)

target_link_libraries(
  cyberverse_platform_tests
  PRIVATE
  cyberverse::platform::static_lib
  Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(cyberverse_platform_tests)
