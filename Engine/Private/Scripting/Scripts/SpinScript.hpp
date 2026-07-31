#pragma once

#include "Scripting/Script.hpp"

class SpinScript final : public Script
{
public:
  void Start() override;
  void Update(float dt) override;
  void OnCollision(EntityId other) override;

private:
  float m_speed = 1.5f;
};

