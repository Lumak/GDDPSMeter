#include <windows.h>
#include <detours.h>
#include "DetourBase.h"
#include "proc.h"

//=============================================================================
//=============================================================================
DetourBase::DetourBase()
{

}

int DetourBase::HookDetour(DetourFnData &detour)
{
    int error = 0;

    detour.realFn_ = (VoidFn)DetourFindFunction(detour.dllName_, detour.mangleName_);

    if (detour.realFn_ == NULL)
    {
        return 1;
    }

    if (detour.detourFn_)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach((PVOID*)&detour.realFn_, detour.detourFn_);

        error = DetourTransactionCommit();
        if (error < 0) error = -error;
    }

    return error;
}
int DetourBase::HookDetour(DetourFnNormalData &detour)
{
    int error = 0;

    detour.realFn_ = (VoidFn)DetourFindFunction(detour.dllName_, detour.mangleName_);

    if (detour.realFn_ == NULL)
    {
        return -1;
    }

    if (detour.detourFn_)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach((PVOID*)&detour.realFn_, detour.detourFn_);
        error = DetourTransactionCommit();
        if (error < 0) error = -error;
    }

    return error;
}

void DetourBase::SetError(const char *err)
{
    DWORD ForegroundWindowProcessID;
    DWORD procId = GetProcId(L"Grim Dawn.exe");
    HWND hwnd = 0;

    if (procId == 0)
    {
        ExitProcess(EXIT_SUCCESS);
    }

    while (true)
    {
        GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);

        if (ForegroundWindowProcessID == procId)
        {
            hwnd = GetForegroundWindow();
            break;
        }
    }

    MessageBoxA(hwnd, err, "Error", MB_OK | MB_ICONERROR);
    ExitProcess(EXIT_SUCCESS);
}
