#include "TrailSpawnerScript.hpp"

#include "EngineApp.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"

#include <cmath>

void TrailSpawnerScript::Start()
{
  m_hasLast = false;
  m_accumDist = 0.0f;
}

void TrailSpawnerScript::Update(float /*dt*/)
{
  if (!m_ctx.world || !m_ctx.app) return;

  const auto* tr = m_ctx.world->tryGet<Transform>(m_ctx.self);
  if (!tr) return;

  const float x = tr->px;
  const float y = tr->py;

  if (!m_hasLast)
  {
    m_lastX = x;
    m_lastY = y;
    m_hasLast = true;
    return;
  }

  const float dx = x - m_lastX;
  const float dy = y - m_lastY;
  const float dist = std::sqrt(dx * dx + dy * dy);
  if (dist < 1e-6f)
  {
    return;
  }

  const float dirX = dx / dist;
  const float dirY = dy / dist;

  float remaining = dist;
  float px = m_lastX;
  float py = m_lastY;

  while (m_accumDist + remaining >= m_spacing)
  {
    const float step = m_spacing - m_accumDist;
    px += dirX * step;
    py += dirY * step;
    remaining -= step;
    m_accumDist = 0.0f;

    // Spawn along the path, not exactly on top of the owner.
    m_ctx.app->spawnTrailSegment(m_ctx.self, px, py, m_segmentSize, m_ttl, m_r, m_g, m_b);
  }

  m_accumDist += remaining;
  m_lastX = x;
  m_lastY = y;
}

