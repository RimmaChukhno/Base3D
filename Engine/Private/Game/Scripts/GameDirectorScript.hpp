#pragma once

#include "Scripting/Script.hpp"

class GameDirectorScript final : public Script
{
public:
  void Start() override;
  void Update(float dt) override;

  void setScoreEntity(EntityId e) { m_scoreEntity = e; }
  void setVictoryTime(float seconds) { m_victoryTime = seconds; }

private:
  EntityId m_scoreEntity{};
  float m_victoryTime = 30.0f;
};

