#include <utils.hpp>

#include <Windows.h>
#include <cassert>
#include <stdio.h>
#include <vector>

HANDLE out, old_out;
HANDLE err, old_err;
HANDLE in, old_in;

namespace utils
{
  void attach_console()
  {
    old_out = GetStdHandle(STD_OUTPUT_HANDLE);
    old_err = GetStdHandle(STD_ERROR_HANDLE);
    old_in = GetStdHandle(STD_INPUT_HANDLE);

    AllocConsole() && AttachConsole(GetCurrentProcessId());

    out = GetStdHandle(STD_OUTPUT_HANDLE);
    err = GetStdHandle(STD_ERROR_HANDLE);
    in = GetStdHandle(STD_INPUT_HANDLE);

    assert(out && err && in);

    SetConsoleMode(out,
      ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

    SetConsoleMode(in,
      ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
      ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
  }
  void detach_console()
  {
    if(out && err && in) {
      FreeConsole();

      if(old_out)
        SetStdHandle(STD_OUTPUT_HANDLE, old_out);
      if(old_err)
        SetStdHandle(STD_ERROR_HANDLE, old_err);
      if(old_in)
        SetStdHandle(STD_INPUT_HANDLE, old_in);
    }
  }
  bool console_print(const char* fmt, ...)
  {
    char buf[1024];
    va_list va;

    va_start(va, fmt);
    _vsnprintf_s(buf, 1024, fmt, va);
    va_end(va);

    return !!WriteConsoleA(out, buf, static_cast<DWORD>(strlen(buf)), NULL, NULL);
  }
  std::wstring console_read()
  {
    wchar_t buf[1024];
    DWORD read;

    ZeroMemory(buf, sizeof(buf));

    SetConsoleMode(in,
      ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT |
      ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
      ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);

    ReadConsoleW(in, buf, sizeof(buf), &read, NULL);

    SetConsoleMode(in,
      ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
      ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);

    return std::wstring(buf, wcslen(buf));
  }
  wchar_t console_read_key()
  {
    wchar_t buf;
    DWORD read;

    ReadConsoleW(in, &buf, 1, &read, NULL);

    return buf;
  }
  int wait_for_modules(std::int32_t timeout, std::initializer_list<std::string> modules)
  {
    bool signaled[32] = { 0 };
    bool success = false;

    std::uint32_t totalSlept = 0;

    if(timeout == 0) {
      for(auto& mod : modules) {
        if(GetModuleHandleA(std::data(mod)) == NULL)
          return WAIT_TIMEOUT;
      }
      return WAIT_OBJECT_0;
    }

    if(timeout < 0)
      timeout = INT32_MAX;

    while(true) {
      for(auto i = 0u; i < modules.size(); ++i) {
        auto& module = *(modules.begin() + i);
        if(!signaled[i] && GetModuleHandleA(std::data(module)) != NULL) {
          signaled[i] = true;

          //
          // Checks if all modules are signaled
          //
          bool done = true;
          for(auto j = 0u; j < modules.size(); ++j) {
            if(!signaled[j]) {
              done = false;
              break;
            }
          }
          if(done) {
            success = true;
            goto exit;
          }
        }
      }
      if(totalSlept > std::uint32_t(timeout)) {
        break;
      }
      Sleep(10);
      totalSlept += 10;
    }

  exit:
    return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
  }

  std::uint8_t* pattern_scan(void* module, const char* signature)
  {
    static auto pattern_to_byte = [](const char* pattern) {
      auto bytes = std::vector<int>{};
      auto start = const_cast<char*>(pattern);
      auto end = const_cast<char*>(pattern) + strlen(pattern);

      for(auto current = start; current < end; ++current) {
        if(*current == '?') {
          ++current;
          if(*current == '?')
            ++current;
          bytes.push_back(-1);
        } else {
          bytes.push_back(strtoul(current, &current, 16));
        }
      }
      return bytes;
    };

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    const auto patternBytes = pattern_to_byte(signature);
    const auto scanBytes = reinterpret_cast<std::uint8_t*>(module);
    
    const auto pattern_len = patternBytes.size();

    for(auto i = 0ul; i < sizeOfImage - pattern_len; ++i) {
      bool found = true;
      for(auto j = 0ul; j < pattern_len; ++j) {
        if(scanBytes[i + j] != patternBytes[j] && patternBytes[j] != -1) {
          found = false;
          break;
        }
      }
      if(found) {
        return &scanBytes[i];
      }
    }
    return nullptr;
  }
}