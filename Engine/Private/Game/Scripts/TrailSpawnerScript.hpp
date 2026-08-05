#pragma once

#include "Scripting/Script.hpp"

class TrailSpawnerScript final : public Script
{
public:
  void Start() override;
  void Update(float dt) override;

  void setColor(float r, float g, float b) { m_r = r; m_g = g; m_b = b; }
  void setSpacing(float s) { m_spacing = s; }
  void setSegmentSize(float s) { m_segmentSize = s; }
  void setTtl(float ttl) { m_ttl = ttl; }

private:
  float m_r = 0.2f;
  float m_g = 1.0f;
  float m_b = 0.4f;

  float m_spacing = 0.05f;     // distance between segments
  float m_segmentSize = 0.03f; // quad scale
  float m_ttl = 10.0f;

  bool m_hasLast = false;
  float m_lastX = 0.0f;
  float m_lastY = 0.0f;
  float m_accumDist = 0.0f;
};

