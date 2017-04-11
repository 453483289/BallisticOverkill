#pragma once

#include <string>
#include <initializer_list>

namespace utils
{
  void attach_console();

  void detach_console();

  bool console_print(const char* fmt, ...);

  std::wstring console_read();

  wchar_t console_read_key();

  int wait_for_modules(std::int32_t timeout, std::initializer_list<std::string> modules);

  template<typename ...Args>
  int wait_for_modules(std::int32_t timeout, Args... modules)
  {
    return wait_for_modules(timeout, { modules... });
  }

  std::uint8_t* pattern_scan(void* module, const char* signature);
}
