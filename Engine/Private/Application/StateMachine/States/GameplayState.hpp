#pragma once

#include "Application/StateMachine/IState.hpp"

class GameplayState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::Gameplay; }
  void update(StateContext& ctx) override;
};

