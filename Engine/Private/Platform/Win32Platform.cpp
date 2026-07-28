#include "Win32Platform.hpp"

#include <Windows.h>

void* Win32Platform::currentModuleHandle()
{
  return reinterpret_cast<void*>(GetModuleHandleW(nullptr));
}

