#pragma once

#include "Scripting/Script.hpp"

class PlayerControllerScript final : public Script
{
public:
  void Start() override;
  void Update(float dt) override;
  void OnCollision(EntityId other) override;

private:
  float m_speed = 0.55f;
  float m_spawnTime = 0.0f;
};

