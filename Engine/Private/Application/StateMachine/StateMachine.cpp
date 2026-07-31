#include "StateMachine.hpp"

#include "Application/StateMachine/States/SplashState.hpp"
#include "Application/StateMachine/States/MainMenuState.hpp"
#include "Application/StateMachine/States/LoadingState.hpp"
#include "Application/StateMachine/States/GameplayState.hpp"
#include "Application/StateMachine/States/PauseState.hpp"
#include "Application/StateMachine/States/GameOverState.hpp"
#include "Application/StateMachine/States/VictoryState.hpp"

#include "EngineApp.hpp"

#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"

void StateMachine::init(EngineApp& app, InputManager& input, TimeSystem& time)
{
  m_app = &app;
  m_input = &input;
  m_time = &time;

  m_ctx = std::make_unique<StateContext>(StateContext{ app, input, time });

  m_currentId = GameStateId::Splash;
  m_state = createState(m_currentId);
  m_state->onEnter(*m_ctx);
}

FramePlan StateMachine::tick()
{
  if (!m_state || !m_ctx) return {};

  // Reset per-frame context.
  m_ctx->plan = FramePlan{};
  m_ctx->wantsTransition = false;
  m_ctx->nextState = m_currentId;

  // State update sets the plan + possibly a transition.
  m_state->update(*m_ctx);

  if (m_ctx->wantsTransition && m_ctx->nextState != m_currentId)
  {
    changeState(m_ctx->nextState);
    // Give the new state a chance to define the plan immediately.
    m_ctx->plan = FramePlan{};
    m_ctx->wantsTransition = false;
    m_ctx->nextState = m_currentId;
    m_state->update(*m_ctx);
  }

  return m_ctx->plan;
}

void StateMachine::changeState(GameStateId next)
{
  if (!m_state || !m_ctx) return;

  m_state->onExit(*m_ctx);
  m_state = createState(next);
  m_currentId = next;
  m_state->onEnter(*m_ctx);
}

std::unique_ptr<IState> StateMachine::createState(GameStateId id)
{
  switch (id)
  {
  case GameStateId::Splash:   return std::make_unique<SplashState>();
  case GameStateId::MainMenu: return std::make_unique<MainMenuState>();
  case GameStateId::Loading:  return std::make_unique<LoadingState>();
  case GameStateId::Gameplay: return std::make_unique<GameplayState>();
  case GameStateId::Pause:    return std::make_unique<PauseState>();
  case GameStateId::GameOver: return std::make_unique<GameOverState>();
  case GameStateId::Victory:  return std::make_unique<VictoryState>();
  default:                    return std::make_unique<SplashState>();
  }
}

