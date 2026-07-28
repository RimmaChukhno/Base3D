#include "TimeSystem.h"

#include <Windows.h>

bool TimeSystem::init()
{
  LARGE_INTEGER freq{};
  if (!QueryPerformanceFrequency(&freq))
  {
    return false;
  }

  m_qpcFreq = freq.QuadPart;

  LARGE_INTEGER now{};
  QueryPerformanceCounter(&now);
  m_lastQpc = now.QuadPart;

  m_deltaSeconds = 0.0f;
  m_totalSeconds = 0.0f;
  m_fps = 0.0f;
  m_fpsAccumSeconds = 0.0f;
  m_fpsFrames = 0;
  m_fixedAccumSeconds = 0.0f;

  return true;
}

void TimeSystem::tick()
{
  LARGE_INTEGER now{};
  QueryPerformanceCounter(&now);

  const long long qpcDelta = now.QuadPart - m_lastQpc;
  m_lastQpc = now.QuadPart;

  const double seconds = (m_qpcFreq > 0) ? (static_cast<double>(qpcDelta) / static_cast<double>(m_qpcFreq)) : 0.0;

  // Clamp to avoid huge dt when debugging/breakpoints.
  m_deltaSeconds = static_cast<float>(seconds);
  if (m_deltaSeconds < 0.0f) m_deltaSeconds = 0.0f;
  if (m_deltaSeconds > 0.25f) m_deltaSeconds = 0.25f;

  m_totalSeconds += m_deltaSeconds;

  // FPS over a short window (0.5s).
  m_fpsAccumSeconds += m_deltaSeconds;
  m_fpsFrames += 1;
  if (m_fpsAccumSeconds >= 0.5f)
  {
    m_fps = static_cast<float>(m_fpsFrames) / m_fpsAccumSeconds;
    m_fpsAccumSeconds = 0.0f;
    m_fpsFrames = 0;
  }

  // Fixed-step accumulation.
  m_fixedAccumSeconds += m_deltaSeconds;
}

void TimeSystem::setFixedDelta(float fixedDeltaSeconds)
{
  if (fixedDeltaSeconds <= 0.0f) return;
  m_fixedDeltaSeconds = fixedDeltaSeconds;
}

int TimeSystem::consumeFixedSteps()
{
  if (m_fixedDeltaSeconds <= 0.0f) return 0;

  int steps = 0;
  while (m_fixedAccumSeconds >= m_fixedDeltaSeconds && steps < 8) // safety cap
  {
    m_fixedAccumSeconds -= m_fixedDeltaSeconds;
    steps++;
  }
  return steps;
}

