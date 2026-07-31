#pragma once

#include "Application/StateMachine/IState.hpp"

class VictoryState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::Victory; }
  void update(StateContext& ctx) override;
};

