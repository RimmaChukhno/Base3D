#pragma once

#include "Application/StateMachine/IState.hpp"

class LoadingState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::Loading; }
  void onEnter(StateContext& ctx) override;
  void update(StateContext& ctx) override;

private:
  float m_enterTime = 0.0f;
};

