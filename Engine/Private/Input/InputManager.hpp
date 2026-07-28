#pragma once

#include <cstdint>

class InputManager
{
public:
  bool init();

  // Feed from the Win32 message pump (forwarded by Game.exe).
  void onWin32Message(uint32_t msg, uintptr_t wParam, intptr_t lParam);

  // Called once per Engine_Tick() to compute pressed/released/deltas.
  void tick();

  bool isKeyDown(uint32_t vk) const;
  bool wasKeyPressed(uint32_t vk) const;
  bool wasKeyReleased(uint32_t vk) const;

  bool isMouseButtonDown(int button) const;   // 0=L,1=R,2=M
  bool wasMouseButtonPressed(int button) const;
  bool wasMouseButtonReleased(int button) const;

  int32_t mouseX() const { return m_mouseX; }
  int32_t mouseY() const { return m_mouseY; }
  int32_t mouseDeltaX() const { return m_mouseDeltaX; }
  int32_t mouseDeltaY() const { return m_mouseDeltaY; }
  int32_t wheelDelta() const { return m_wheelDelta; }

private:
  void clearAll();

private:
  bool m_keys[256]{};
  bool m_prevKeys[256]{};
  bool m_pressed[256]{};
  bool m_released[256]{};

  bool m_mouseButtons[3]{};
  bool m_prevMouseButtons[3]{};
  bool m_mousePressed[3]{};
  bool m_mouseReleased[3]{};

  int32_t m_mouseX = 0;
  int32_t m_mouseY = 0;
  int32_t m_prevMouseX = 0;
  int32_t m_prevMouseY = 0;
  int32_t m_mouseDeltaX = 0;
  int32_t m_mouseDeltaY = 0;

  int32_t m_wheelAccum = 0;
  int32_t m_prevWheelAccum = 0;
  int32_t m_wheelDelta = 0;
};

