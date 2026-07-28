#include "Win32Platform.h"

#include <Windows.h>

void* Win32Platform::currentModuleHandle()
{
  return reinterpret_cast<void*>(GetModuleHandleW(nullptr));
}

