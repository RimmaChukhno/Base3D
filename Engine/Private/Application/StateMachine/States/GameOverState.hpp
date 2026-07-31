#pragma once

#include "Application/StateMachine/IState.hpp"

class GameOverState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::GameOver; }
  void update(StateContext& ctx) override;
};

