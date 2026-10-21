cmake_minimum_required(VERSION 4.0.0)

include(FetchContent)

# GLM is header-only; don't build its library, tests, or install rules. Resolve an
# existing target first so the library can be embedded in a larger build.
if(NOT TARGET glm::glm)
  set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
  set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW ON
  )
  FetchContent_MakeAvailable(glm)
endif()

if(CYBERVERSE_PLATFORM_BACKEND_GLFW AND NOT TARGET glfw)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
  )
  FetchContent_MakeAvailable(glfw)
endif()

add_library(cyberverse_platform_static_lib STATIC)
add_library(cyberverse::platform::static_lib ALIAS cyberverse_platform_static_lib)

target_include_directories(cyberverse_platform_static_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Public headers
set(
  HEADER_FILES
  cyberverse/platform.hpp
  cyberverse/platform/backend.hpp
  cyberverse/platform/key_mapping.hpp
  cyberverse/platform/platform.hpp
  cyberverse/platform/raw_input.hpp
  cyberverse/platform/timer.hpp
  cyberverse/platform/window.hpp
  cyberverse/platform/detail/frame_timer.hpp
  cyberverse/platform/detail/release_guard.hpp
  cyberverse/platform/backends/headless/platform.hpp
  cyberverse/platform/backends/headless/window.hpp
)

# Source files
set(
  SOURCE_FILES
  cyberverse/platform/platform.cpp
  cyberverse/platform/timer.cpp
  cyberverse/platform/window.cpp
  cyberverse/platform/raw_input.cpp
  cyberverse/platform/key_mapping.cpp
  cyberverse/platform/detail/frame_timer.cpp
  cyberverse/platform/backends/headless/platform.cpp
  cyberverse/platform/backends/headless/window.cpp
)

if(CYBERVERSE_PLATFORM_BACKEND_GLFW)
  list(
    APPEND
    HEADER_FILES
    cyberverse/platform/backends/glfw/glfw.hpp
    cyberverse/platform/backends/glfw/native.hpp
    cyberverse/platform/backends/glfw/platform.hpp
    cyberverse/platform/backends/glfw/window.hpp
    cyberverse/platform/backends/glfw/macos/window.hpp
  )
  list(
    APPEND
    SOURCE_FILES
    cyberverse/platform/backends/glfw/platform.cpp
    cyberverse/platform/backends/glfw/window.cpp
  )

  if(APPLE)
    list(APPEND SOURCE_FILES cyberverse/platform/backends/glfw/macos/window.mm)
    set_source_files_properties(
      cyberverse/platform/backends/glfw/macos/window.mm
      PROPERTIES
        LANGUAGE OBJCXX
        COMPILE_OPTIONS "-fobjc-arc"
    )
  else()
    list(APPEND SOURCE_FILES cyberverse/platform/backends/glfw/native.cpp)
  endif()
endif()

target_sources(cyberverse_platform_static_lib PUBLIC ${HEADER_FILES})
target_sources(cyberverse_platform_static_lib PRIVATE ${SOURCE_FILES})

target_link_libraries(cyberverse_platform_static_lib PUBLIC glm::glm)

if(CYBERVERSE_PLATFORM_BACKEND_GLFW)
  target_link_libraries(cyberverse_platform_static_lib PUBLIC glfw)
  target_compile_definitions(cyberverse_platform_static_lib PUBLIC CYBERVERSE_PLATFORM_BACKEND_GLFW=1)
endif()

target_compile_definitions(cyberverse_platform_static_lib PUBLIC CYBERVERSE_PLATFORM_BACKEND_HEADLESS=1)

if(APPLE AND CYBERVERSE_PLATFORM_BACKEND_GLFW)
  target_link_libraries(
    cyberverse_platform_static_lib
    PUBLIC
    "-framework Cocoa"
    "-framework QuartzCore"
  )
endif()
