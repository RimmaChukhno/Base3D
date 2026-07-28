#include "InputManager.h"

#include <Windows.h>
#include <Windowsx.h>

#include <algorithm>
#include <cstring>

bool InputManager::init()
{
  clearAll();
  return true;
}

void InputManager::clearAll()
{
  std::memset(m_keys, 0, sizeof(m_keys));
  std::memset(m_prevKeys, 0, sizeof(m_prevKeys));
  std::memset(m_pressed, 0, sizeof(m_pressed));
  std::memset(m_released, 0, sizeof(m_released));

  std::memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
  std::memset(m_prevMouseButtons, 0, sizeof(m_prevMouseButtons));
  std::memset(m_mousePressed, 0, sizeof(m_mousePressed));
  std::memset(m_mouseReleased, 0, sizeof(m_mouseReleased));

  m_mouseX = m_mouseY = 0;
  m_prevMouseX = m_prevMouseY = 0;
  m_mouseDeltaX = m_mouseDeltaY = 0;

  m_wheelAccum = 0;
  m_prevWheelAccum = 0;
  m_wheelDelta = 0;
}

static uint32_t clampVk(uintptr_t wParam)
{
  const uint32_t vk = static_cast<uint32_t>(wParam & 0xFFFFu);
  return std::min<uint32_t>(vk, 255u);
}

void InputManager::onWin32Message(uint32_t msg, uintptr_t wParam, intptr_t lParam)
{
  switch (msg)
  {
  case WM_KILLFOCUS:
    clearAll();
    break;

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  {
    const uint32_t vk = clampVk(wParam);
    m_keys[vk] = true;
    break;
  }
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    const uint32_t vk = clampVk(wParam);
    m_keys[vk] = false;
    break;
  }

  case WM_LBUTTONDOWN: m_mouseButtons[0] = true; break;
  case WM_LBUTTONUP:   m_mouseButtons[0] = false; break;
  case WM_RBUTTONDOWN: m_mouseButtons[1] = true; break;
  case WM_RBUTTONUP:   m_mouseButtons[1] = false; break;
  case WM_MBUTTONDOWN: m_mouseButtons[2] = true; break;
  case WM_MBUTTONUP:   m_mouseButtons[2] = false; break;

  case WM_MOUSEMOVE:
    m_mouseX = GET_X_LPARAM(lParam);
    m_mouseY = GET_Y_LPARAM(lParam);
    break;

  case WM_MOUSEWHEEL:
  {
    const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
    // Accumulate in "wheel notches" (WHEEL_DELTA=120).
    m_wheelAccum += delta / WHEEL_DELTA;
    break;
  }

  default:
    break;
  }
}

void InputManager::tick()
{
  for (int i = 0; i < 256; ++i)
  {
    m_pressed[i] = (m_keys[i] && !m_prevKeys[i]);
    m_released[i] = (!m_keys[i] && m_prevKeys[i]);
    m_prevKeys[i] = m_keys[i];
  }

  for (int b = 0; b < 3; ++b)
  {
    m_mousePressed[b] = (m_mouseButtons[b] && !m_prevMouseButtons[b]);
    m_mouseReleased[b] = (!m_mouseButtons[b] && m_prevMouseButtons[b]);
    m_prevMouseButtons[b] = m_mouseButtons[b];
  }

  m_mouseDeltaX = m_mouseX - m_prevMouseX;
  m_mouseDeltaY = m_mouseY - m_prevMouseY;
  m_prevMouseX = m_mouseX;
  m_prevMouseY = m_mouseY;

  m_wheelDelta = m_wheelAccum - m_prevWheelAccum;
  m_prevWheelAccum = m_wheelAccum;
}

bool InputManager::isKeyDown(uint32_t vk) const
{
  return (vk < 256) ? m_keys[vk] : false;
}

bool InputManager::wasKeyPressed(uint32_t vk) const
{
  return (vk < 256) ? m_pressed[vk] : false;
}

bool InputManager::wasKeyReleased(uint32_t vk) const
{
  return (vk < 256) ? m_released[vk] : false;
}

bool InputManager::isMouseButtonDown(int button) const
{
  return (button >= 0 && button < 3) ? m_mouseButtons[button] : false;
}

bool InputManager::wasMouseButtonPressed(int button) const
{
  return (button >= 0 && button < 3) ? m_mousePressed[button] : false;
}

bool InputManager::wasMouseButtonReleased(int button) const
{
  return (button >= 0 && button < 3) ? m_mouseReleased[button] : false;
}

