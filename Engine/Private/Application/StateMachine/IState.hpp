#pragma once

#include "Application/StateMachine/GameStateId.hpp"

class EngineApp;
class InputManager;
class TimeSystem;

struct FramePlan
{
  bool runScripts = false;
  bool runMotion = false;
  bool runCollision = false;
  bool runRender = true;
};

struct StateContext
{
  EngineApp& app;
  InputManager& input;
  TimeSystem& time;
  FramePlan plan{};

  // Transition request (set by states).
  bool wantsTransition = false;
  GameStateId nextState = GameStateId::Splash;

  void transition(GameStateId id)
  {
    wantsTransition = true;
    nextState = id;
  }
};

class IState
{
public:
  virtual ~IState() = default;

  virtual GameStateId id() const = 0;
  virtual void onEnter(StateContext& /*ctx*/) {}
  virtual void onExit(StateContext& /*ctx*/) {}
  virtual void update(StateContext& ctx) = 0;
};

