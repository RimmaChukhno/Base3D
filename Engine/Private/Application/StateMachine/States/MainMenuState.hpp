#pragma once

#include "Application/StateMachine/IState.hpp"

class MainMenuState final : public IState
{
public:
  GameStateId id() const override { return GameStateId::MainMenu; }
  void onEnter(StateContext& ctx) override;
  void update(StateContext& ctx) override;

private:
  int m_selected = 0; // 0=Start, 1=Exit
};

