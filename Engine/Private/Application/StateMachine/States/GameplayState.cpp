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
  ctx.plan.runParticles = true;
  ctx.plan.runRender = true;

  // Game result transitions (Step 11).
  if (ctx.app.consumeGameOverRequested())
  {
    ctx.transition(GameStateId::GameOver);
    return;
  }
  if (ctx.app.consumeVictoryRequested())
  {
    ctx.transition(GameStateId::Victory);
    return;
  }

  // P toggles pause.
  if (ctx.input.wasKeyPressed('P'))
  {
    ctx.transition(GameStateId::Pause);
    return;
  }

  // ESC exits
  if (ctx.input.wasKeyPressed(VK_ESCAPE))
  {
    ctx.app.requestQuit();
  }
}

