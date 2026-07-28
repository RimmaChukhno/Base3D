#pragma once

// Intentionally tiny for early steps.
// We keep Win32 types out of Engine/Public to preserve a clean DLL boundary.

struct Win32Platform
{
  static void* currentModuleHandle();
};

