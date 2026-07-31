#pragma once

#include "Application/StateMachine/IState.hpp"

class SplashState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::Splash; }
  void onEnter(StateContext& ctx) override;
  void update(StateContext& ctx) override;

private:
  float m_enterTime = 0.0f;
};

