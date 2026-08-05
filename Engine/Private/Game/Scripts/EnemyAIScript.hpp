#pragma once

#include "Scripting/Script.hpp"

class EnemyAIScript final : public Script
{
public:
  void Update(float dt) override;

  // Set by spawner/builder
  void setTarget(EntityId player) { m_player = player; }

private:
  EntityId m_player{};
  float m_speed = 0.45f;
};

