#include "PlayerControllerScript.hpp"

#include "EngineApp.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"

#include "Game/Components/WallTag.hpp"
#include "Game/Components/EnemyTag.hpp"
#include "Game/Components/TrailTag.hpp"
#include "Game/Components/TrailSegment.hpp"

#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"

#include <Windows.h>
#include <cmath>

static float clampf(float v, float a, float b) { return (v < a) ? a : (v > b) ? b : v; }

void PlayerControllerScript::Start()
{
  m_spawnTime = m_ctx.time ? m_ctx.time->totalSeconds() : 0.0f;
}

void PlayerControllerScript::Update(float dt)
{
  if (!m_ctx.world || !m_ctx.input) return;

  auto* tr = m_ctx.world->tryGet<Transform>(m_ctx.self);
  if (!tr) return;

  float dx = 0.0f;
  float dy = 0.0f;

  if (m_ctx.input->isKeyDown('W') || m_ctx.input->isKeyDown(VK_UP)) dy += 1.0f;
  if (m_ctx.input->isKeyDown('S') || m_ctx.input->isKeyDown(VK_DOWN)) dy -= 1.0f;
  if (m_ctx.input->isKeyDown('D') || m_ctx.input->isKeyDown(VK_RIGHT)) dx += 1.0f;
  if (m_ctx.input->isKeyDown('A') || m_ctx.input->isKeyDown(VK_LEFT)) dx -= 1.0f;

  // Normalize diagonal movement (cheap).
  const float lenSq = dx * dx + dy * dy;
  if (lenSq > 1e-6f)
  {
    const float invLen = 1.0f / std::sqrt(lenSq);
    dx *= invLen;
    dy *= invLen;
  }

  tr->px += dx * m_speed * dt;
  tr->py += dy * m_speed * dt;

  // Clamp inside arena bounds (keeps gameplay stable).
  tr->px = clampf(tr->px, -0.90f, 0.90f);
  tr->py = clampf(tr->py, -0.90f, 0.90f);
}

void PlayerControllerScript::OnCollision(EntityId other)
{
  if (!m_ctx.world || !m_ctx.app) return;

  // Small grace window after spawn so early overlaps
  // don't create instant, unfair failures.
  const float now = m_ctx.time ? m_ctx.time->totalSeconds() : 0.0f;
  const float spawnGrace = 1.25f;
  const bool inSpawnGrace = (now - m_spawnTime) < spawnGrace;

  // Trail rules:
  // - hitting any trail => game over
  // - except a short grace window on your own freshly spawned segments
  if (m_ctx.world->has<TrailTag>(other))
  {
    const auto* seg = m_ctx.world->tryGet<TrailSegment>(other);
    if (inSpawnGrace)
    {
      return;
    }
    if (seg && seg->owner == m_ctx.self)
    {
      const float grace = 0.85f;
      if ((now - seg->spawnTime) < grace)
      {
        return; // ignore immediate self-collision
      }
    }
    m_ctx.app->requestGameOver();
    return;
  }

  // Game rules (Step 11.1):
  // player colliding with walls or enemies => game over.
  if (m_ctx.world->has<WallTag>(other) || (!inSpawnGrace && m_ctx.world->has<EnemyTag>(other)))
  {
    m_ctx.app->requestGameOver();
  }
}

