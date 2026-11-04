/**
 * @file system_info.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Cross-platform system, memory, and process information
 * @version 0.1
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace cyberverse {
namespace platform {

/**
 * @brief Static-ish host system information.
 *
 * These values describe the machine rather than a window, renderer, or backend.
 * Queries are best effort: a platform may leave a field empty or zero when the
 * OS does not expose it.
 */
struct system_info {
  std::string os_name;                   ///< "macOS", "Windows", "Linux", or the uname sysname
  std::string os_version;                ///< OS release/build string
  std::string architecture;              ///< "arm64", "x86_64", ...
  std::string cpu_model;                 ///< CPU brand string; empty when unavailable
  std::uint32_t logical_core_count = 0;  ///< Online logical processors
  std::uint32_t physical_core_count = 0; ///< Physical cores; 0 when unavailable
  std::uint64_t total_memory_bytes = 0;  ///< Installed physical memory
};

/**
 * @brief Dynamic host memory information.
 */
struct memory_info {
  std::uint64_t available_memory_bytes = 0; ///< Currently available physical memory
};

/**
 * @brief Raw cumulative CPU time counters.
 *
 * Units are OS scheduler ticks. The counters are monotonic while the machine is
 * running; callers compute utilization from two snapshots.
 */
struct cpu_times {
  std::uint64_t total_ticks = 0; ///< Total non-idle and idle CPU ticks
  std::uint64_t idle_ticks = 0;  ///< Idle CPU ticks
};

/**
 * @brief Memory usage of the calling process.
 */
struct process_info {
  std::uint64_t resident_bytes = 0; ///< Resident set size
  std::uint64_t virtual_bytes = 0;  ///< Virtual address space size
};

} // namespace platform
} // namespace cyberverse
