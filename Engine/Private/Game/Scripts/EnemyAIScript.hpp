#pragma once

#include "Scripting/Script.hpp"

class EnemyAIScript final : public Script
{
public:
  void Start() override;
  void Update(float dt) override;
  void OnCollision(EntityId other) override;

  // Set by spawner/builder
  void setTarget(EntityId player) { m_player = player; }

private:
  EntityId m_player{};
  float m_speed = 0.14f;
  uint32_t m_rng = 0xA5EED123u;

  // Simple, cheap "avoidance" response so we don't teleport every frame.
  float m_spawnTime = 0.0f;
  float m_lastReactTime = -1000.0f;
  float m_reactCooldown = 0.18f;

  float m_avoidTimer = 0.0f;
  float m_avoidX = 0.0f;
  float m_avoidY = 0.0f;
  float m_avoidSpeed = 0.40f;
};

