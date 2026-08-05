#include "MainMenuState.hpp"

#include "Input/InputManager.hpp"
#include "EngineApp.hpp"

#include <Windows.h>

void MainMenuState::onEnter(StateContext& ctx)
{
  // Ensure no stale gameplay flags.
  (void)ctx.app.consumeGameOverRequested();
  (void)ctx.app.consumeVictoryRequested();
}

void MainMenuState::update(StateContext& ctx)
{
  // Menu: freeze gameplay, keep rendering background.
  ctx.plan.runRender = true;

  if (ctx.input.wasKeyPressed(VK_RETURN))
  {
    ctx.transition(GameStateId::Loading);
    return;
  }

  // Escape quits the app (convenient for validation).
  if (ctx.input.wasKeyPressed(VK_ESCAPE))
  {
    ctx.app.requestQuit();
  }
}

