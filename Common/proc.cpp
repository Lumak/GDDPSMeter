//
// Copyright (c) 2022 the GDDPSMeter project.
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
          break;
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

//=============================================================================
// reference for this code is from:
// https://5paceman.dev/dll-injecting-getprocaddress-of-a-remote-process/
//=============================================================================
uintptr_t GetProcAddressRemote(DWORD procId, BOOL is64BitProcess, const wchar_t* modName, std::string &functionName)
{
	uintptr_t modHandle = 0;
	uintptr_t remoteProcAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	MODULEENTRY32W modEntry;

	if (hSnap != INVALID_HANDLE_VALUE)
	{
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

	//error check
	if (modHandle == 0)
	{
		return 0;
	}

	uintptr_t moduleBase = (uintptr_t)(modEntry.modBaseAddr);
	DWORD moduleSize = modEntry.dwSize;

	if (!moduleBase || !moduleSize)
	{
		return 0;
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

	if (hProc)
	{
		IMAGE_DOS_HEADER dosHeader = {};
		ReadProcessMemory(hProc, (LPVOID)(moduleBase), &dosHeader, sizeof(IMAGE_DOS_HEADER), nullptr);

		if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
		{
			return 0;
		}

		uintptr_t exportDirectoryVA = 0;

		if (is64BitProcess == FALSE)
		{
			IMAGE_NT_HEADERS32 ntHeader = {};
			ReadProcessMemory(hProc, (LPVOID)(moduleBase + dosHeader.e_lfanew), &ntHeader, sizeof(IMAGE_NT_HEADERS32), nullptr);

			if (ntHeader.Signature != IMAGE_NT_SIGNATURE)
			{
				return 0;
			}

			if (!ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
			{
				return 0;
			}

			exportDirectoryVA = ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		}
		else 
		{
			IMAGE_NT_HEADERS64 ntHeader = {};
			ReadProcessMemory(hProc, (LPVOID)(moduleBase + dosHeader.e_lfanew), &ntHeader, sizeof(IMAGE_NT_HEADERS64), nullptr);

			if (ntHeader.Signature != IMAGE_NT_SIGNATURE)
			{
				return 0;
			}

			if (!ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
			{
				return 0;
			}

			exportDirectoryVA = ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		}

		IMAGE_EXPORT_DIRECTORY expDirectory = {};
		ReadProcessMemory(hProc, (LPVOID)(moduleBase + exportDirectoryVA), &expDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), nullptr);

		uintptr_t addrOfFunc = moduleBase + expDirectory.AddressOfFunctions;
		uintptr_t addrOfNames = moduleBase + expDirectory.AddressOfNames;
		uintptr_t addrOfOrdinals = moduleBase + expDirectory.AddressOfNameOrdinals;

		WORD ordinal = 0;
		size_t len_buf = functionName.length() + 1;
		char* nameBuff = new char[len_buf];
		size_t addrLength = (is64BitProcess == TRUE ? sizeof(DWORD) : sizeof(uintptr_t));

		for (DWORD i = 0; i < expDirectory.NumberOfNames; i++)
		{
			uintptr_t rvaString = 0;
			ReadProcessMemory(hProc, (LPVOID)(addrOfNames + (i * addrLength)), &rvaString, addrLength, nullptr);
			ReadProcessMemory(hProc, (LPVOID)(moduleBase + rvaString), nameBuff, len_buf, nullptr);

			if (!lstrcmpiA(functionName.c_str(), nameBuff))
			{
				ReadProcessMemory(hProc, (LPVOID)(addrOfOrdinals + (i * sizeof(WORD))), &ordinal, sizeof(WORD), nullptr);
				uintptr_t funcRVA = 0;
				ReadProcessMemory(hProc, (LPVOID)(addrOfFunc + (ordinal * addrLength)), &funcRVA, addrLength, nullptr);
				remoteProcAddr = moduleBase + funcRVA;
				break;
			}
		}
		delete[] nameBuff;

		CloseHandle(hProc);
	}

	return remoteProcAddr;
}
