#include "VictoryState.hpp"

#include "Input/InputManager.hpp"
#include "EngineApp.hpp"

#include <Windows.h>

void VictoryState::update(StateContext& ctx)
{
  ctx.plan.runRender = true;

  if (ctx.input.wasKeyPressed(VK_RETURN))
  {
    ctx.transition(GameStateId::MainMenu);
    return;
  }

  if (ctx.input.wasKeyPressed(VK_ESCAPE))
  {
    ctx.app.requestQuit();
  }
}

