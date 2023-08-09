#pragma once
#include <vector>
#include "Logger.h"

//=============================================================================
//=============================================================================
namespace DetourUtil
{
  bool IsPlayerInWorld(void* player);
  char *WStr2CharStr(const char *format, ...);
  void DumpHex(unsigned int *ptr, int len);
  bool MemValidity(void *memPtr);
  void GetBackTrace();

  template<class T>
  void VectorMemoryToVector0(unsigned int addr, std::vector<T> &vec)
  {
    // some vector memory seen in detours sent from grim dawn is shown as:
    // 0xaddr0, 0xaddr1, 0xaddr2, ..., 0xaddrn
    // the 0xaddr0 = begin()
    // the 0xaddr1 = end() **note the 0xaddr2 seems to be equal to 0xaddr1 (always?)
    // and just increment 0xaddr0 by sizeof(T) until you reach the end()
    unsigned int esi = 0;
    unsigned int edi = 0;
    unsigned int ecx = 0;

    edi = addr;
    esi = *(unsigned int*)edi;
    while (1)
    {
      if (esi == *(unsigned int*)(edi + sizeof(T)))
      {
        break;
      }

      ecx = *(unsigned int*)esi;
      vec.push_back((T)ecx);
      esi += 4;
    }
  }

}
