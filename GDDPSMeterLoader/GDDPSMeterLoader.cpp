#include <windows.h>
#include <stdio.h>
#include "proc.h"

//=============================================================================
//=============================================================================
#define GRIMDAWN L"Grim Dawn.exe"
#define DPSMETERDLL L"DPSMeter.dll"

//=============================================================================
//=============================================================================
void PopMessage(bool isError, const char *msg)
{
    if (isError)
    {
        MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBoxA(NULL, msg, "Success", MB_OK | MB_ICONINFORMATION);
    }
}

BOOL FileExists(LPCSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

//=============================================================================
//=============================================================================
int main()
{
    DWORD dwProcessId = 0;
    bool found = false;
    bool dllLoaded = false;
    bool hideConsole = true;

    if (hideConsole)
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    dwProcessId = GetProcId(GRIMDAWN);

    if (dwProcessId != 0)
    {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess)
        {
            //no reload
            if (GetBaseModuleHandle(dwProcessId, DPSMETERDLL))
            {
                PopMessage(true, "GD DPSMeter is already loaded.");
                return 0;
            }

            const int maxPathLen = 512;
            char path[maxPathLen];
            GetCurrentDirectory(maxPathLen, path);

#ifndef _RELEASE
            std::string dllPath = std::string(path) + "\\" + CMAKE_INTDIR + "\\DPSMeter.dll";
#else
            std::string dllPath = std::string(path) + "\\DPSMeter.dll";
#endif

            LPCSTR DllPath = dllPath.c_str();

			//check for dll
			if (!FileExists(DllPath))
			{
				PopMessage(true, "DPSMeter.dll missing.");
				return 0;
			}

            LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            if (pDllPath != NULL)
            {
                BOOL success = WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);
                while (!success)
                {
                    Sleep(100);
                    success = WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);
                }

                HMODULE hKern32 = GetModuleHandleA("Kernel32.dll");

                if (hKern32 != NULL && hKern32 != INVALID_HANDLE_VALUE)
                {
                    LPTHREAD_START_ROUTINE pthread = (LPTHREAD_START_ROUTINE)GetProcAddress(hKern32, "LoadLibraryA");
                    HANDLE hLoadThread = CreateRemoteThread(hProcess, 0, 0, pthread, pDllPath, 0, 0);

                    if (hLoadThread)
                    {
                        // wait for the execution to finish
                        WaitForSingleObject(hLoadThread, INFINITE);

                        // close LoadLibrary thread (dll is already loaded into host process)
                        CloseHandle(hLoadThread);
                        dllLoaded = true;
                    }
                    else
                    {
                        PopMessage(true, "Remote thread failed.");
                    }
                }
                else
                {
                    PopMessage(true, "Failed to load Kernel32.dll.");
                }

                VirtualFreeEx(hProcess, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);
            }
            else
            {
                PopMessage(true, "Failed to inject dll.");
            }
            CloseHandle(hProcess);
        }
        else
        {
            PopMessage(true, "OpenProcess failed.");
        }
    }
    else
    {
        PopMessage(true, "Failed to find \"Grim Dawn.exe\" process.");
    }

    return 0;
}

