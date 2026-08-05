#include "MainMenuState.hpp"

#include "Input/InputManager.hpp"
#include "EngineApp.hpp"

#include <Windows.h>

void MainMenuState::onEnter(StateContext& ctx)
{
  // Ensure no stale gameplay flags.
  (void)ctx.app.consumeGameOverRequested();
  (void)ctx.app.consumeVictoryRequested();

  m_selected = 0;
  ctx.app.prepareMenuWorld();
  ctx.app.setMenuSelection(m_selected);
}

void MainMenuState::update(StateContext& ctx)
{
  // Menu: freeze gameplay, keep rendering background.
  ctx.plan.runRender = true;

  if (ctx.input.wasKeyPressed(VK_UP) || ctx.input.wasKeyPressed('W'))
  {
    m_selected = 0;
    ctx.app.setMenuSelection(m_selected);
  }
  if (ctx.input.wasKeyPressed(VK_DOWN) || ctx.input.wasKeyPressed('S'))
  {
    m_selected = 1;
    ctx.app.setMenuSelection(m_selected);
  }

  if (ctx.input.wasKeyPressed(VK_RETURN))
  {
    if (m_selected == 0)
    {
      ctx.transition(GameStateId::Loading);
    }
    else
    {
      ctx.app.requestQuit();
    }
    return;
  }

  // Escape quits the app (convenient for validation).
  if (ctx.input.wasKeyPressed(VK_ESCAPE))
  {
    ctx.app.requestQuit();
  }
}

