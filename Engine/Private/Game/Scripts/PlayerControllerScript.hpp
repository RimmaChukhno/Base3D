#pragma once

#include "Scripting/Script.hpp"

class PlayerControllerScript final : public Script
{
public:
  void Update(float dt) override;
  void OnCollision(EntityId other) override;

private:
  float m_speed = 0.8f;
};

