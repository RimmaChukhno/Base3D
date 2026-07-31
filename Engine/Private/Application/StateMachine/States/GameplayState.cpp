#include "GameplayState.hpp"

#include "Input/InputManager.hpp"
#include "EngineApp.hpp"

#include <Windows.h>

void GameplayState::update(StateContext& ctx)
{
  // Full engine pipeline.
  ctx.plan.runScripts = true;
  ctx.plan.runMotion = true;
  ctx.plan.runCollision = true;
  ctx.plan.runRender = true;

  // P toggles pause.
  if (ctx.input.wasKeyPressed('P'))
  {
    ctx.transition(GameStateId::Pause);
    return;
  }

  // Debug transitions for validation (temporary):
  // - K => Victory
  // - L => GameOver
  if (ctx.input.wasKeyPressed('K'))
  {
    ctx.transition(GameStateId::Victory);
    return;
  }
  if (ctx.input.wasKeyPressed('L'))
  {
    ctx.transition(GameStateId::GameOver);
    return;
  }

  // ESC exits
  if (ctx.input.wasKeyPressed(VK_ESCAPE))
  {
    ctx.app.requestQuit();
  }
}

