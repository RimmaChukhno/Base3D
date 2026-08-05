#include "GameDirectorScript.hpp"

#include "EngineApp.hpp"

#include "ECS/World.hpp"
#include "Game/Components/ScoreComponent.hpp"

#include <cmath>

void GameDirectorScript::Start()
{
  // Reset score on Start.
  if (!m_ctx.world) return;
  if (auto* s = m_ctx.world->tryGet<ScoreComponent>(m_scoreEntity))
  {
    s->timeAlive = 0.0f;
    s->score = 0;
  }
}

void GameDirectorScript::Update(float dt)
{
  if (!m_ctx.world || !m_ctx.app) return;

  auto* s = m_ctx.world->tryGet<ScoreComponent>(m_scoreEntity);
  if (!s) return;

  s->timeAlive += dt;
  s->score = static_cast<int>(std::floor(s->timeAlive * 10.0f));

  if (s->timeAlive >= m_victoryTime)
  {
    m_ctx.app->requestVictory();
  }
}

