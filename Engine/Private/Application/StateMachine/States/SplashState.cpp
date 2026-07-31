#include "SplashState.hpp"

#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"

#include <Windows.h>

void SplashState::onEnter(StateContext& ctx)
{
  m_enterTime = ctx.time.totalSeconds();
  (void)ctx;
}

void SplashState::update(StateContext& ctx)
{
  // Splash: show something briefly, then go to menu.
  ctx.plan.runRender = true;

  // Skip splash if user presses any "start-ish" key.
  if (ctx.input.wasKeyPressed(VK_RETURN) || ctx.input.wasKeyPressed(VK_SPACE))
  {
    ctx.transition(GameStateId::MainMenu);
    return;
  }

  const float elapsed = ctx.time.totalSeconds() - m_enterTime;
  if (elapsed >= 1.5f)
  {
    ctx.transition(GameStateId::MainMenu);
  }
}

