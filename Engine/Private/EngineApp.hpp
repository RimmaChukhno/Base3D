#pragma once

#include "EngineAPI.hpp"

#include <cstdint>
#include <string>

class D3D11Renderer;
class InputManager;
class TimeSystem;

class EngineApp
{
public:
  EngineApp();
  ~EngineApp();

  EngineApp(const EngineApp&) = delete;
  EngineApp& operator=(const EngineApp&) = delete;

  bool init(const EngineConfig& cfg);
  void tick();
  void onResize(int32_t width, int32_t height);
  void onWin32Message(uint32_t msg, uintptr_t wParam, intptr_t lParam);
  void getFrameStats(EngineFrameStats& outStats) const;
  void shutdown();

private:
  EngineConfig m_cfg{};
  std::wstring m_assetsRoot;
  EngineFrameStats m_frameStats{};
  D3D11Renderer* m_renderer = nullptr;
  InputManager* m_input = nullptr;
  TimeSystem* m_time = nullptr;
};

