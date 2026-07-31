#include "PauseState.hpp"

#include "Input/InputManager.hpp"

#include <Windows.h>

void PauseState::update(StateContext& ctx)
{
  // Pause: no gameplay simulation, only render.
  ctx.plan.runRender = true;

  // P resumes.
  if (ctx.input.wasKeyPressed('P') || ctx.input.wasKeyPressed(VK_ESCAPE))
  {
    ctx.transition(GameStateId::Gameplay);
    return;
  }

  // Enter returns to menu.
  if (ctx.input.wasKeyPressed(VK_RETURN))
  {
    ctx.transition(GameStateId::MainMenu);
  }
}

