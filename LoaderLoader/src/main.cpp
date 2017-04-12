#include <Windows.h>
#include <cstdint>
#include <TlHelp32.h>
#include <stdio.h>

bool check_file_exists(const char* file)
{
  HANDLE file_handle = CreateFileA(file, GENERIC_READ | SYNCHRONIZE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if(file_handle == INVALID_HANDLE_VALUE) return false;

  CloseHandle(file_handle);
  return true;
}

void* open_process(const wchar_t* name)
{
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if(snap == INVALID_HANDLE_VALUE) {
    SetLastError(ERROR_OBJECT_NOT_FOUND);
    return nullptr;
  }

  PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };

  if(Process32First(snap, &pe)) {
    do {
      if(!_wcsicmp(name, pe.szExeFile)) {
        CloseHandle(snap);
        return OpenProcess(
          PROCESS_CREATE_THREAD |
          PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
          PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
      }
    } while(Process32Next(snap, &pe));
  }
  CloseHandle(snap);
  return nullptr;
}

void* alloc_remote_buffer(void* process, size_t size, bool executable)
{
  return VirtualAllocEx(process, nullptr, size, MEM_COMMIT | MEM_RESERVE, executable ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
}

void free_remote_buffer(void* process, void* address)
{
  VirtualFreeEx(process, address, 0, MEM_RELEASE);
}

void enable_debug_priv()
{
  HANDLE process_token;
  HANDLE local_process;
  TOKEN_PRIVILEGES tp;

  local_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
  LookupPrivilegeValue(NULL, TEXT("SeDebugPrivilege"), &tp.Privileges[0].Luid);

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  if(
    !OpenProcessToken(local_process, TOKEN_ADJUST_PRIVILEGES, &process_token) ||
    !AdjustTokenPrivileges(process_token, FALSE, &tp, NULL, NULL, NULL)
    ) {
    fprintf(stderr, "AdjustTokenPrivileges failed. The applciation will try to keep running but it could fail!\n");
  }

  CloseHandle(process_token);
  CloseHandle(local_process);
}

int main()
{
  char load_path[MAX_PATH];

  memset(load_path, 0, sizeof(load_path));
  _fullpath(load_path, "bo_loader.dll", sizeof(load_path));

  if(!check_file_exists(load_path))
    return ERROR_FILE_NOT_FOUND;

  enable_debug_priv();

  auto proc_handle = open_process(L"BallisticOverkill.exe");

  if(!proc_handle)
    return GetLastError();

  auto buffer = alloc_remote_buffer(proc_handle, 260, false);

  if(!buffer)
    return GetLastError();

  if(!WriteProcessMemory(proc_handle, buffer, load_path, sizeof(load_path), nullptr)) {
    free_remote_buffer(proc_handle, buffer);
    return GetLastError();
  }

  auto loadLibrary = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");

  auto thread = CreateRemoteThread(
    proc_handle,
    nullptr, 0,
    (LPTHREAD_START_ROUTINE)loadLibrary,
    buffer,
    0, nullptr);

  WaitForSingleObject(thread, 5000);
  free_remote_buffer(proc_handle, buffer);
  return ERROR_SUCCESS;
}