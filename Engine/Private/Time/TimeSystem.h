#pragma once

class TimeSystem
{
public:
  bool init();
  void tick();

  float deltaSeconds() const { return m_deltaSeconds; }
  float totalSeconds() const { return m_totalSeconds; }
  float fps() const { return m_fps; }

  // Fixed-step scaffold (used later for physics).
  void setFixedDelta(float fixedDeltaSeconds);
  int consumeFixedSteps(); // returns how many fixed updates to run this frame
  float fixedDelta() const { return m_fixedDeltaSeconds; }

private:
  long long m_qpcFreq = 0;
  long long m_lastQpc = 0;

  float m_deltaSeconds = 0.0f;
  float m_totalSeconds = 0.0f;

  // FPS smoothing (simple rolling window).
  float m_fps = 0.0f;
  float m_fpsAccumSeconds = 0.0f;
  int m_fpsFrames = 0;

  // Fixed-step.
  float m_fixedDeltaSeconds = 1.0f / 60.0f;
  float m_fixedAccumSeconds = 0.0f;
};

