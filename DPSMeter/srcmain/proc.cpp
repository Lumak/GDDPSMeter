//
// Copyright (c) 2022 the TQModRuntime project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "proc.h"
#include <psapi.h>

//=============================================================================
// reference for this routine was found in this video:
// https://www.youtube.com/watch?v=wiX5LmdD5yk
//=============================================================================
DWORD GetProcId(const wchar_t* procName)
{
  DWORD procId = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    PROCESSENTRY32W procEntry;
    procEntry.dwSize = sizeof(procEntry);

    if (Process32FirstW(hSnap, &procEntry))
    {
      do
      {
        if (!_wcsicmp(procEntry.szExeFile, procName))
        {
          procId = procEntry.th32ProcessID;
          break;
        }
      } while (Process32NextW(hSnap, &procEntry));
    }
  }

  CloseHandle(hSnap);

  return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
  uintptr_t modBaseAddr = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    MODULEENTRY32W modEntry;
    modEntry.dwSize = sizeof(modEntry);

    if (Module32FirstW(hSnap, &modEntry))
    {
      do
      {
        if (!_wcsicmp(modEntry.szModule, modName))
        {
          modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
          //break;
        }
      } while (Module32NextW(hSnap, &modEntry));
    }
  }

  CloseHandle(hSnap);

  return modBaseAddr;
}

uintptr_t GetBaseModuleHandle(DWORD procId, const wchar_t* modName)
{
  uintptr_t modHandle = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    MODULEENTRY32W modEntry;
    modEntry.dwSize = sizeof(modEntry);

    if (Module32FirstW(hSnap, &modEntry))
    {
      do
      {
        if (!_wcsicmp(modEntry.szModule, modName))
        {
          modHandle = (uintptr_t)modEntry.hModule;
          break;
        }
      } while (Module32NextW(hSnap, &modEntry));
    }
  }

  CloseHandle(hSnap);

  return modHandle;
}

DWORD GetThreadId(DWORD procId)
{
  DWORD threadId = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, procId);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    THREADENTRY32 th32;
    th32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hSnap, &th32))
    {
      do
      {
        if (th32.th32OwnerProcessID == procId)
        {
          threadId = th32.th32ThreadID;
          break;
        }
      } while (Thread32Next(hSnap, &th32));
    }
  }

  CloseHandle(hSnap);

  return threadId;
}

bool SuspendResumeProcess(DWORD processId, bool suspend)
{
  HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

  THREADENTRY32 threadEntry;
  bool success = false;
  threadEntry.dwSize = sizeof(THREADENTRY32);

  Thread32First(hThreadSnapshot, &threadEntry);

  do
  {
    if (threadEntry.th32OwnerProcessID == processId)
    {
      HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
        threadEntry.th32ThreadID);

      if (suspend)
      {
        SuspendThread(hThread);
      }
      else
      {
        ResumeThread(hThread);
      }
      success = true;
      CloseHandle(hThread);
    }
  } while (Thread32Next(hThreadSnapshot, &threadEntry));

  CloseHandle(hThreadSnapshot);

  return success;
}

BOOL ModulePathContainsPattern(DWORD procId, const wchar_t* modName, const wchar_t* pattern)
{
  BOOL hasPattern = FALSE;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    MODULEENTRY32W modEntry;
    modEntry.dwSize = sizeof(modEntry);

    if (Module32FirstW(hSnap, &modEntry))
    {
      do
      {
        if (!_wcsicmp(modEntry.szModule, modName))
        {
          if (wcsstr(modEntry.szExePath, pattern) != NULL)
            hasPattern = TRUE;
          break;
        }
      } while (Module32NextW(hSnap, &modEntry));
    }
  }

  CloseHandle(hSnap);

  return hasPattern;
}


