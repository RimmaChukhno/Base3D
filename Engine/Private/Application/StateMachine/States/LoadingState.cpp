#include "LoadingState.hpp"

#include "Time/TimeSystem.hpp"
#include "EngineApp.hpp"

void LoadingState::onEnter(StateContext& ctx)
{
  m_enterTime = ctx.time.totalSeconds();
  ctx.app.prepareGameplayWorld();
}

void LoadingState::update(StateContext& ctx)
{
  ctx.plan.runRender = true;

  // Simulate a short loading time.
  if (ctx.time.totalSeconds() - m_enterTime >= 1.0f)
  {
    ctx.transition(GameStateId::Gameplay);
  }
}

