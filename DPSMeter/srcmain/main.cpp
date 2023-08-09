#include <windows.h>
#include "DetourMain.h"


BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID lptr)
{
    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        DetourMain::Init();
        break;

    case DLL_PROCESS_DETACH:
        DetourMain::Close();
        break;
    }

    return TRUE;
}

