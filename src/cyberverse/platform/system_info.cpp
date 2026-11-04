/**
 * @file system_info.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Cross-platform system, memory, and process information
 * @version 0.1
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/platform/system_info.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <thread>

#include "cyberverse/platform/platform.hpp"

#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#include <mach/task.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <unistd.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <fstream>
#include <set>
#include <sstream>
#include <utility>
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <psapi.h>
#include <winreg.h>

#include <vector>
#endif

namespace cyberverse {
namespace platform {
namespace {

std::uint32_t fallback_logical_cores() noexcept {
  const unsigned count = std::thread::hardware_concurrency();
  return count == 0 ? 0 : static_cast<std::uint32_t>(count);
}

#if defined(__APPLE__)

std::string sysctl_string(const char* name) {
  std::size_t size = 0;
  if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0) {
    return {};
  }

  std::string value(size, '\0');
  if (sysctlbyname(name, value.data(), &size, nullptr, 0) != 0) {
    return {};
  }
  while (!value.empty() && value.back() == '\0') {
    value.pop_back();
  }
  return value;
}

std::uint64_t sysctl_u64(const char* name, std::uint64_t fallback = 0) noexcept {
  std::uint64_t value = 0;
  std::size_t size = sizeof(value);
  if (sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
    return fallback;
  }
  return value;
}

std::uint32_t sysctl_u32(const char* name, std::uint32_t fallback = 0) noexcept {
  std::uint32_t value = 0;
  std::size_t size = sizeof(value);
  if (sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
    return fallback;
  }
  return value;
}

system_info query_system_info_impl() {
  system_info info;
  info.os_name = "macOS";

  utsname names{};
  if (uname(&names) == 0) {
    info.os_version = names.release;
    info.architecture = names.machine;
  }

  info.cpu_model = sysctl_string("machdep.cpu.brand_string");
  info.logical_core_count = sysctl_u32("hw.logicalcpu", fallback_logical_cores());
  info.physical_core_count = sysctl_u32("hw.physicalcpu", 0);
  info.total_memory_bytes = sysctl_u64("hw.memsize", 0);
  return info;
}

memory_info query_memory_info_impl() noexcept {
  memory_info info;
  vm_statistics64_data_t stats{};
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(mach_host_self(), HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&stats), &count) !=
      KERN_SUCCESS) {
    return info;
  }

  vm_size_t page_size = 0;
  if (host_page_size(mach_host_self(), &page_size) != KERN_SUCCESS) {
    return info;
  }

  const std::uint64_t pages =
      static_cast<std::uint64_t>(stats.free_count) + static_cast<std::uint64_t>(stats.inactive_count);
  info.available_memory_bytes = pages * static_cast<std::uint64_t>(page_size);
  return info;
}

std::optional<cpu_times> query_cpu_times_impl() {
  natural_t cpu_count = 0;
  processor_info_array_t info = nullptr;
  mach_msg_type_number_t info_count = 0;
  if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &cpu_count, &info, &info_count) != KERN_SUCCESS) {
    return std::nullopt;
  }

  std::uint64_t total = 0;
  std::uint64_t idle = 0;
  const auto* loads = reinterpret_cast<processor_cpu_load_info_t>(info);
  for (natural_t index = 0; index < cpu_count; ++index) {
    for (int state = 0; state < CPU_STATE_MAX; ++state) {
      const std::uint64_t ticks = loads[index].cpu_ticks[state];
      total += ticks;
      if (state == CPU_STATE_IDLE) {
        idle += ticks;
      }
    }
  }

  vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(info), info_count * sizeof(integer_t));
  if (total == 0) {
    return std::nullopt;
  }
  return cpu_times{ total, idle };
}

process_info query_process_info_impl() noexcept {
  process_info info;
  mach_task_basic_info_data_t basic{};
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&basic), &count) !=
      KERN_SUCCESS) {
    return info;
  }
  info.resident_bytes = static_cast<std::uint64_t>(basic.resident_size);
  info.virtual_bytes = static_cast<std::uint64_t>(basic.virtual_size);
  return info;
}

#elif defined(__linux__)

std::string first_matching_line(const char* path, const std::string& prefix) {
  std::ifstream stream(path);
  std::string line;
  while (std::getline(stream, line)) {
    if (line.rfind(prefix, 0) == 0) {
      return line;
    }
  }
  return {};
}

int parse_int_value(std::string_view line) noexcept {
  const auto separator = line.find(':');
  if (separator == std::string_view::npos) {
    return -1;
  }

  std::size_t begin = separator + 1;
  while (begin < line.size() && (line[begin] == ' ' || line[begin] == '\t')) {
    ++begin;
  }

  int value = 0;
  bool seen = false;
  while (begin < line.size() && line[begin] >= '0' && line[begin] <= '9') {
    value = value * 10 + (line[begin] - '0');
    ++begin;
    seen = true;
  }
  return seen ? value : -1;
}

std::uint64_t parse_kib_value(std::string_view line) noexcept {
  const auto separator = line.find(':');
  if (separator == std::string_view::npos) {
    return 0;
  }

  std::size_t begin = separator + 1;
  while (begin < line.size() && (line[begin] == ' ' || line[begin] == '\t')) {
    ++begin;
  }

  std::uint64_t value = 0;
  while (begin < line.size() && line[begin] >= '0' && line[begin] <= '9') {
    value = value * 10 + static_cast<std::uint64_t>(line[begin] - '0');
    ++begin;
  }
  return value * 1024;
}

system_info query_system_info_impl() {
  system_info info;
  info.os_name = "Linux";

  utsname names{};
  if (uname(&names) == 0) {
    info.os_version = names.release;
    info.architecture = names.machine;
  }

  const long online = sysconf(_SC_NPROCESSORS_ONLN);
  info.logical_core_count = online > 0 ? static_cast<std::uint32_t>(online) : fallback_logical_cores();

  const std::string model = first_matching_line("/proc/cpuinfo", "model name");
  const auto colon = model.find(':');
  if (colon != std::string::npos) {
    std::size_t begin = colon + 1;
    while (begin < model.size() && model[begin] == ' ') {
      ++begin;
    }
    info.cpu_model = model.substr(begin);
  }

  const auto total_line = first_matching_line("/proc/meminfo", "MemTotal:");
  info.total_memory_bytes = parse_kib_value(total_line);

  std::ifstream cpuinfo("/proc/cpuinfo");
  std::set<std::pair<int, int>> physical_cores;
  int physical_id = -1;
  int core_id = -1;
  std::string line;
  while (std::getline(cpuinfo, line)) {
    if (line.rfind("physical id", 0) == 0) {
      physical_id = parse_int_value(line);
    } else if (line.rfind("core id", 0) == 0) {
      core_id = parse_int_value(line);
    } else if (line.empty()) {
      if (physical_id >= 0 && core_id >= 0) {
        physical_cores.emplace(physical_id, core_id);
      }
      physical_id = -1;
      core_id = -1;
    }
  }
  if (physical_id >= 0 && core_id >= 0) {
    physical_cores.emplace(physical_id, core_id);
  }
  info.physical_core_count = static_cast<std::uint32_t>(physical_cores.size());
  return info;
}

std::optional<cpu_times> query_cpu_times_impl() {
  std::ifstream stream("/proc/stat");
  std::string line;
  if (!std::getline(stream, line) || line.rfind("cpu", 0) != 0) {
    return std::nullopt;
  }

  std::istringstream values(line);
  std::string label;
  values >> label;
  std::uint64_t total = 0;
  std::uint64_t idle = 0;
  std::uint64_t value = 0;
  int index = 0;
  while (values >> value) {
    total += value;
    if (index == 3 || index == 4) {
      idle += value;
    }
    ++index;
  }
  if (total == 0) {
    return std::nullopt;
  }
  return cpu_times{ total, idle };
}

memory_info query_memory_info_impl() noexcept {
  memory_info info;
  const auto line = first_matching_line("/proc/meminfo", "MemAvailable:");
  info.available_memory_bytes = parse_kib_value(line);
  return info;
}

process_info query_process_info_impl() noexcept {
  process_info info;

  const std::string resident = first_matching_line("/proc/self/status", "VmRSS:");
  const std::string virtual_size = first_matching_line("/proc/self/status", "VmSize:");
  info.resident_bytes = parse_kib_value(resident);
  info.virtual_bytes = parse_kib_value(virtual_size);
  return info;
}

#elif defined(_WIN32)

std::string windows_error_architecture(WORD architecture) {
  switch (architecture) {
  case PROCESSOR_ARCHITECTURE_AMD64:
    return "x86_64";
  case PROCESSOR_ARCHITECTURE_ARM64:
    return "arm64";
  case PROCESSOR_ARCHITECTURE_ARM:
    return "arm";
  case PROCESSOR_ARCHITECTURE_INTEL:
    return "x86";
  default:
    return "unknown";
  }
}

std::uint64_t filetime_ticks(const FILETIME& value) noexcept {
  return (static_cast<std::uint64_t>(value.dwHighDateTime) << 32) | static_cast<std::uint64_t>(value.dwLowDateTime);
}

std::optional<cpu_times> query_cpu_times_impl() noexcept {
  FILETIME idle_time{};
  FILETIME kernel_time{};
  FILETIME user_time{};
  if (GetSystemTimes(&idle_time, &kernel_time, &user_time) == 0) {
    return std::nullopt;
  }

  const std::uint64_t idle = filetime_ticks(idle_time);
  const std::uint64_t total = filetime_ticks(kernel_time) + filetime_ticks(user_time);
  if (total == 0) {
    return std::nullopt;
  }
  return cpu_times{ total, idle };
}

std::string registry_string(const char* key, const char* name) {
  DWORD size = 0;
  if (RegGetValueA(HKEY_LOCAL_MACHINE, key, name, RRF_RT_REG_SZ, nullptr, nullptr, &size) != ERROR_SUCCESS ||
      size == 0) {
    return {};
  }

  std::string value(size, '\0');
  if (RegGetValueA(HKEY_LOCAL_MACHINE, key, name, RRF_RT_REG_SZ, nullptr, value.data(), &size) != ERROR_SUCCESS) {
    return {};
  }
  while (!value.empty() && value.back() == '\0') {
    value.pop_back();
  }
  return value;
}

std::uint32_t registry_dword(const char* key, const char* name) {
  DWORD value = 0;
  DWORD size = sizeof(value);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, key, name, RRF_RT_REG_DWORD, nullptr, &value, &size) != ERROR_SUCCESS) {
    return 0;
  }
  return value;
}

std::string registry_cpu_model() {
  return registry_string("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString");
}

std::string windows_version() {
  constexpr const char* key = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
  const std::uint32_t major = registry_dword(key, "CurrentMajorVersionNumber");
  const std::uint32_t minor = registry_dword(key, "CurrentMinorVersionNumber");
  const std::string build = registry_string(key, "CurrentBuild");

  if (major == 0) {
    return build;
  }

  std::string version = std::to_string(major) + "." + std::to_string(minor);
  if (!build.empty()) {
    version += "." + build;
  }
  return version;
}

std::uint32_t physical_core_count() {
  DWORD size = 0;
  if (GetLogicalProcessorInformation(nullptr, &size) != 0 || size == 0) {
    return 0;
  }

  std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> entries(size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
  if (GetLogicalProcessorInformation(entries.data(), &size) == 0) {
    return 0;
  }

  std::uint32_t count = 0;
  for (const auto& entry : entries) {
    if (entry.Relationship == RelationProcessorCore) {
      ++count;
    }
  }
  return count;
}

system_info query_system_info_impl() {
  system_info info;
  info.os_name = "Windows";

  SYSTEM_INFO system{};
  GetNativeSystemInfo(&system);
  info.architecture = windows_error_architecture(system.wProcessorArchitecture);
  info.logical_core_count = system.dwNumberOfProcessors != 0 ? static_cast<std::uint32_t>(system.dwNumberOfProcessors)
                                                             : fallback_logical_cores();
  info.physical_core_count = physical_core_count();

  info.os_version = windows_version();
  info.cpu_model = registry_cpu_model();

  MEMORYSTATUSEX memory{};
  memory.dwLength = sizeof(memory);
  if (GlobalMemoryStatusEx(&memory) != 0) {
    info.total_memory_bytes = memory.ullTotalPhys;
  }
  return info;
}

memory_info query_memory_info_impl() noexcept {
  memory_info info;
  MEMORYSTATUSEX memory{};
  memory.dwLength = sizeof(memory);
  if (GlobalMemoryStatusEx(&memory) != 0) {
    info.available_memory_bytes = memory.ullAvailPhys;
  }
  return info;
}

process_info query_process_info_impl() noexcept {
  process_info info;
  PROCESS_MEMORY_COUNTERS_EX counters{};
  if (GetProcessMemoryInfo(
          GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters)) != 0) {
    info.resident_bytes = static_cast<std::uint64_t>(counters.WorkingSetSize);
    info.virtual_bytes = static_cast<std::uint64_t>(counters.PrivateUsage);
  }
  return info;
}

#else

system_info query_system_info_impl() {
  system_info info;
  info.os_name = "Unknown";
  info.logical_core_count = fallback_logical_cores();
  return info;
}

memory_info query_memory_info_impl() noexcept { return {}; }
process_info query_process_info_impl() noexcept { return {}; }
std::optional<cpu_times> query_cpu_times_impl() noexcept { return std::nullopt; }

#endif

} // namespace

system_info platform::query_system_info() const { return query_system_info_impl(); }

memory_info platform::query_memory_info() const { return query_memory_info_impl(); }

process_info platform::query_process_info() const { return query_process_info_impl(); }

std::optional<cpu_times> platform::query_cpu_times() const { return query_cpu_times_impl(); }

} // namespace platform
} // namespace cyberverse
