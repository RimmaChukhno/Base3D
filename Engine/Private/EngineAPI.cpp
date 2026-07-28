#include "EngineAPI.hpp"

#include "EngineApp.hpp"

struct EngineHandle
{
  EngineApp app;
};

static bool isValidConfig(const EngineConfig* cfg)
{
  if (!cfg) return false;
  if (!cfg->windowHandle) return false;
  if (cfg->width <= 0 || cfg->height <= 0) return false;
  if (!cfg->assetsRoot) return false;
  return true;
}

extern "C" ENGINE_API EngineHandle* Engine_Create(const EngineConfig* cfg)
{
  if (!isValidConfig(cfg))
  {
    return nullptr;
  }

  EngineHandle* handle = new EngineHandle{};
  if (!handle->app.init(*cfg))
  {
    delete handle;
    return nullptr;
  }

  return handle;
}

extern "C" ENGINE_API void Engine_Destroy(EngineHandle* engine)
{
  if (!engine) return;
  engine->app.shutdown();
  delete engine;
}

extern "C" ENGINE_API void Engine_Tick(EngineHandle* engine)
{
  if (!engine) return;
  engine->app.tick();
}

extern "C" ENGINE_API void Engine_Resize(EngineHandle* engine, int32_t width, int32_t height)
{
  if (!engine) return;
  if (width <= 0 || height <= 0) return;
  engine->app.onResize(width, height);
}

extern "C" ENGINE_API void Engine_OnWin32Message(EngineHandle* engine, uint32_t msg, uintptr_t wParam, intptr_t lParam)
{
  if (!engine) return;
  engine->app.onWin32Message(msg, wParam, lParam);
}

extern "C" ENGINE_API void Engine_GetFrameStats(const EngineHandle* engine, EngineFrameStats* outStats)
{
  if (!engine || !outStats) return;
  engine->app.getFrameStats(*outStats);
}

