#pragma once

#include <cstdint>

#if defined(_WIN32)
  #if defined(ENGINE_BUILD_DLL)
    #define ENGINE_API __declspec(dllexport)
  #else
    #define ENGINE_API __declspec(dllimport)
  #endif
#else
  #define ENGINE_API
#endif

extern "C" {

struct EngineFrameStats
{
  float deltaSeconds;
  float totalSeconds;
  float fps;
  int32_t entityCount;
  int32_t movingCount;
  int32_t drawCount;
  int32_t particleVertexCount;
  int32_t collisionPairs;
  int32_t collisionOverlaps;
  int32_t stateId;
  int32_t resMeshes;
  int32_t resMaterials;
  int32_t resShaders;
  int32_t resTextures;
  int32_t mouseX;
  int32_t mouseY;
  int32_t mouseDeltaX;
  int32_t mouseDeltaY;
  int32_t wheelDelta;
};

struct EngineConfig
{
  void* windowHandle; // HWND (kept opaque in the public API)
  int32_t width;
  int32_t height;
  bool enableValidation;
  const wchar_t* assetsRoot; // absolute path preferred (copied internally by Engine)
};

struct EngineHandle; // Opaque handle owned by Engine.dll

ENGINE_API EngineHandle* Engine_Create(const EngineConfig* cfg);
ENGINE_API void Engine_Destroy(EngineHandle* engine);
ENGINE_API void Engine_Tick(EngineHandle* engine);
ENGINE_API void Engine_Resize(EngineHandle* engine, int32_t width, int32_t height);
ENGINE_API void Engine_OnWin32Message(EngineHandle* engine, uint32_t msg, uintptr_t wParam, intptr_t lParam);
ENGINE_API void Engine_GetFrameStats(const EngineHandle* engine, EngineFrameStats* outStats);

}

