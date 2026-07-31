#include "LoadingState.hpp"

#include "Time/TimeSystem.hpp"

void LoadingState::onEnter(StateContext& ctx)
{
  m_enterTime = ctx.time.totalSeconds();
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

