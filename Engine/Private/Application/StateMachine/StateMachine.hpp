#pragma once

#include "Application/StateMachine/GameStateId.hpp"
#include "Application/StateMachine/IState.hpp"

#include <memory>

class EngineApp;
class InputManager;
class TimeSystem;

class StateMachine
{
public:
  void init(EngineApp& app, InputManager& input, TimeSystem& time);

  // Returns the frame plan for this tick (what pipeline stages should run).
  FramePlan tick();

  GameStateId current() const { return m_currentId; }

private:
  void changeState(GameStateId next);
  std::unique_ptr<IState> createState(GameStateId id);

private:
  EngineApp* m_app = nullptr;
  InputManager* m_input = nullptr;
  TimeSystem* m_time = nullptr;

  GameStateId m_currentId = GameStateId::Splash;
  std::unique_ptr<IState> m_state;

  // Shared context object reused each tick (allocated on init).
  std::unique_ptr<StateContext> m_ctx;
};

