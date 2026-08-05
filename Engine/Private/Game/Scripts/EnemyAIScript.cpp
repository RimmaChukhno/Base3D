#include "EnemyAIScript.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"

#include <cmath>

static float clampf(float v, float a, float b) { return (v < a) ? a : (v > b) ? b : v; }

void EnemyAIScript::Update(float dt)
{
  if (!m_ctx.world) return;
  if (m_player == kInvalidEntity) return;

  auto* tr = m_ctx.world->tryGet<Transform>(m_ctx.self);
  const auto* pt = m_ctx.world->tryGet<Transform>(m_player);
  if (!tr || !pt) return;

  float dx = pt->px - tr->px;
  float dy = pt->py - tr->py;

  const float lenSq = dx * dx + dy * dy;
  if (lenSq > 1e-6f)
  {
    const float invLen = 1.0f / std::sqrt(lenSq);
    dx *= invLen;
    dy *= invLen;
  }

  tr->px += dx * m_speed * dt;
  tr->py += dy * m_speed * dt;
  tr->px = clampf(tr->px, -0.92f, 0.92f);
  tr->py = clampf(tr->py, -0.92f, 0.92f);
}

