#pragma once

#include "Application/StateMachine/IState.hpp"

class PauseState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::Pause; }
  void update(StateContext& ctx) override;
};

