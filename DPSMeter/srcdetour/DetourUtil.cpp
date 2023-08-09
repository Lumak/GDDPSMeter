#include <windows.h>
#include <dbghelp.h>
#include <sstream>
#include <iostream>

#include "DetourUtil.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
#define MAX_TRACE_DEPTH 15
#define MAX_FNNAME_LEN  64

//=============================================================================
//=============================================================================
namespace DetourUtil
{
    //NOTE: some of the functions here queries/looks at specific address offsets
    //found via DumpHex. if any of the grim dawn dll's change then these functions must
    //be reverified
    bool IsPlayerInWorld(void* player)
    {
        bool inworld = false;

        if (!player)
        {
            return inworld;
        }

        unsigned int *ucplay = (unsigned int*)player;
        // div 4 bc of unsigned int ptr, if char ptr then /4 is not needed
        unsigned int *zoneinfo = ucplay + 0x9c / 4;
        unsigned int zoneInfoIndir = *zoneinfo;
        unsigned int *zoneregion = (unsigned int*)zoneInfoIndir + 0x34 / 4;
        unsigned int zoneregionContent = *zoneregion;

        // if zone is _PlayerRegion then it's in char loading screen
        if (zoneregionContent != 0x616C505F) //first 4 bytes of "_PlayerRegion" in reverse
        {
            inworld = true;
        }

        return inworld;
    }

    char *WStr2CharStr(const char *format, ...)
    {
        const int buffSize = 256;
        static char buf[buffSize];
        memset(buf, 0, buffSize);

        va_list args;
        va_start(args, format);
        vsprintf_s(buf, format, args);
        va_end(args);
        return buf;
    }

    void DumpHex(unsigned int *ptr, int len)
    {
        //printing per line -- 256 should suffice, 
        //largest possible even for x64: 8 x 16(%I64X) = 128 + spaces
        const int buffSize = 256;
        char buff[buffSize];

        if (ptr)
        {
            for (int i = 0; i < len; i += 8)
            {
                int bufpos = 0;
                for (int j = 0; j < 8 && j + i < len; ++j)
                {
                    bufpos += sprintf_s(buff + bufpos, buffSize - bufpos, "%08X ", *ptr++);
                }
                buff[bufpos] = '\0';
                LOGF("%s\n", buff);
            }
            LOGF("\n");
        }
        else
        {
            LOGF("DumpHex NULL ptr\n");
        }
    }

    bool MemValidity(void *memPtr)
    {
        HANDLE process = GetCurrentProcess();
        MEMORY_BASIC_INFORMATION info;

        if ((unsigned int)memPtr < 0x10000)
        {
            return false;
        }

        bool valid = VirtualQueryEx(process, (LPCVOID)memPtr, &info, sizeof(info)) == sizeof(info);
        return (valid && info.Protect == PAGE_READWRITE && info.State == MEM_COMMIT);
    }

    void GetBackTrace()
    {
#ifndef _RELEASE
        DLOG(LogStackTrace, "----- BackTrace -----\n");

        const int maxFrames = MAX_TRACE_DEPTH;
        const int maxFnNameLen = MAX_FNNAME_LEN;
        void *stack[maxFrames];

        char symBuff[sizeof(SYMBOL_INFO) + maxFnNameLen * sizeof(TCHAR)];
        SYMBOL_INFO *symbol = (PSYMBOL_INFO)symBuff;
        symbol->MaxNameLen = maxFnNameLen;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        HANDLE proc = GetCurrentProcess();
        WORD numFrames = CaptureStackBackTrace(0, maxFrames, stack, NULL);
        SymInitialize(proc, NULL, TRUE);

        for (unsigned i = 0; i < numFrames; ++i)
        {
            DWORD64 addr = (DWORD64)stack[i];

            if (SymFromAddr(proc, addr, NULL, symbol))
            {
                std::stringstream stream;
                stream << "\t" << std::hex << symbol->Address << " - " << symbol->Name << std::endl;
                std::string str = stream.str();
                DLOG(LogStackTrace, "%s", str.c_str());
           }
        }
#endif
    }
}

