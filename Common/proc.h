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

#pragma once

#include <windows.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <iostream>


DWORD GetProcId(const wchar_t* procName);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
BOOL ModulePathContainsPattern(DWORD procId, const wchar_t* modName, const wchar_t* pattern);
uintptr_t GetBaseModuleHandle(DWORD procId, const wchar_t* modName);
DWORD GetThreadId(DWORD procId);
bool SuspendResumeProcess(DWORD processId, bool suspend);
uintptr_t GetProcAddressRemote(DWORD procId, BOOL is64BitProcess, const wchar_t* modName, std::string &functionName);


