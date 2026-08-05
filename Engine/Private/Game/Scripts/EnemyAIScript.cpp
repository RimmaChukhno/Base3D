#include "EnemyAIScript.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"

#include "Game/Components/WallTag.hpp"
#include "Game/Components/TrailTag.hpp"

#include "Time/TimeSystem.hpp"

#include <cmath>

static float clampf(float v, float a, float b) { return (v < a) ? a : (v > b) ? b : v; }

static float rand01(uint32_t& state)
{
  // xorshift32
  uint32_t x = state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  state = x;
  return static_cast<float>(x) / static_cast<float>(0xFFFFFFFFu);
}

void EnemyAIScript::Start()
{
  m_spawnTime = m_ctx.time ? m_ctx.time->totalSeconds() : 0.0f;
  m_lastReactTime = -1000.0f;
  m_avoidTimer = 0.0f;
  m_avoidX = 0.0f;
  m_avoidY = 0.0f;
}

void EnemyAIScript::Update(float dt)
{
  if (!m_ctx.world) return;
  if (m_player == kInvalidEntity) return;

  auto* tr = m_ctx.world->tryGet<Transform>(m_ctx.self);
  const auto* pt = m_ctx.world->tryGet<Transform>(m_player);
  if (!tr || !pt) return;

  // If we recently collided, apply a short avoidance push to get unstuck.
  if (m_avoidTimer > 0.0f)
  {
    tr->px += m_avoidX * m_avoidSpeed * dt;
    tr->py += m_avoidY * m_avoidSpeed * dt;
    m_avoidTimer -= dt;
  }
  else
  {
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
  }
  tr->px = clampf(tr->px, -0.92f, 0.92f);
  tr->py = clampf(tr->py, -0.92f, 0.92f);
}

void EnemyAIScript::OnCollision(EntityId other)
{
  if (!m_ctx.world) return;
  auto* tr = m_ctx.world->tryGet<Transform>(m_ctx.self);
  if (!tr) return;

  const float now = m_ctx.time ? m_ctx.time->totalSeconds() : 0.0f;

  // Short grace after spawn so we don't immediately react to overlaps.
  if ((now - m_spawnTime) < 0.75f) return;

  // Avoid spamming reactions every frame (CollisionSystem reports overlaps continuously).
  if ((now - m_lastReactTime) < m_reactCooldown) return;

  const bool hitWall = m_ctx.world->has<WallTag>(other);
  const bool hitTrail = m_ctx.world->has<TrailTag>(other);
  if (!hitWall && !hitTrail) return;

  float ax = 0.0f;
  float ay = 0.0f;

  if (const auto* ot = m_ctx.world->tryGet<Transform>(other))
  {
    // Push away from the overlapped object.
    ax = tr->px - ot->px;
    ay = tr->py - ot->py;
  }
  else
  {
    // If we don't have a transform (rare), push toward center.
    ax = -tr->px;
    ay = -tr->py;
  }

  float lenSq = ax * ax + ay * ay;
  if (lenSq < 1e-6f)
  {
    // Degenerate direction: pick a stable random direction.
    const float a = (rand01(m_rng) * 6.2831853f);
    ax = std::cos(a);
    ay = std::sin(a);
    lenSq = 1.0f;
  }

  const float invLen = 1.0f / std::sqrt(lenSq);
  ax *= invLen;
  ay *= invLen;

  // Stronger push for wall hits to avoid "sticking".
  m_avoidX = ax;
  m_avoidY = ay;
  m_avoidTimer = hitWall ? 0.22f : 0.14f;
  m_lastReactTime = now;
}

