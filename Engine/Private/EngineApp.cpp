#include "EngineApp.hpp"

#include "Renderer/D3D11Renderer.hpp"
#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"

#include <Windows.h>

EngineApp::EngineApp() = default;

EngineApp::~EngineApp()
{
  shutdown();
}

bool EngineApp::init(const EngineConfig& cfg)
{
  m_cfg = cfg;
  m_assetsRoot = cfg.assetsRoot ? cfg.assetsRoot : L"";
  m_cfg.assetsRoot = m_assetsRoot.c_str();

  if (m_renderer)
  {
    return true;
  }

  m_time = new TimeSystem();
  if (!m_time->init())
  {
    delete m_time;
    m_time = nullptr;
    return false;
  }

  m_input = new InputManager();
  if (!m_input->init())
  {
    delete m_input;
    m_input = nullptr;
    delete m_time;
    m_time = nullptr;
    return false;
  }

  m_renderer = new D3D11Renderer();
  if (!m_renderer->init(m_cfg))
  {
    delete m_renderer;
    m_renderer = nullptr;
    delete m_input;
    m_input = nullptr;
    delete m_time;
    m_time = nullptr;
    return false;
  }

  return true;
}

void EngineApp::tick()
{
  if (!m_renderer || !m_input || !m_time) return;

  m_time->tick();
  m_input->tick();

  // Example controls for validation:
  // - ESC closes the window (proves key pressed works).
  // - Mouse position affects triangle translation (proves mouse + resize path).
  if (m_input->wasKeyPressed(VK_ESCAPE))
  {
    PostMessageW(reinterpret_cast<HWND>(m_cfg.windowHandle), WM_CLOSE, 0, 0);
  }

  float nx = 0.0f;
  float ny = 0.0f;
  if (m_cfg.width > 0 && m_cfg.height > 0)
  {
    nx = (static_cast<float>(m_input->mouseX()) / static_cast<float>(m_cfg.width)) * 2.0f - 1.0f;
    ny = 1.0f - (static_cast<float>(m_input->mouseY()) / static_cast<float>(m_cfg.height)) * 2.0f;
  }

  // Update stats for external debugging (Game window title).
  m_frameStats.deltaSeconds = m_time->deltaSeconds();
  m_frameStats.totalSeconds = m_time->totalSeconds();
  m_frameStats.fps = m_time->fps();
  m_frameStats.mouseX = m_input->mouseX();
  m_frameStats.mouseY = m_input->mouseY();
  m_frameStats.mouseDeltaX = m_input->mouseDeltaX();
  m_frameStats.mouseDeltaY = m_input->mouseDeltaY();
  m_frameStats.wheelDelta = m_input->wheelDelta();

  m_renderer->beginFrame();
  m_renderer->clear(0.05f, 0.10f, 0.20f, 1.0f);
  m_renderer->drawTestTriangle(m_time->totalSeconds(), nx * 0.5f, ny * 0.5f);
  m_renderer->endFrame();
  m_renderer->present();
}

void EngineApp::onResize(int32_t width, int32_t height)
{
  if (!m_renderer) return;
  m_cfg.width = width;
  m_cfg.height = height;
  m_renderer->resize(width, height);
}

void EngineApp::onWin32Message(uint32_t msg, uintptr_t wParam, intptr_t lParam)
{
  if (!m_input) return;
  m_input->onWin32Message(msg, wParam, lParam);
}

void EngineApp::getFrameStats(EngineFrameStats& outStats) const
{
  outStats = m_frameStats;
}

void EngineApp::shutdown()
{
  if (m_renderer)
  {
    m_renderer->shutdown();
    delete m_renderer;
    m_renderer = nullptr;
  }

  delete m_input;
  m_input = nullptr;

  delete m_time;
  m_time = nullptr;
}

