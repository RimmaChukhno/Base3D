#include "PlayerControllerScript.hpp"

#include "EngineApp.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"

#include "Game/Components/WallTag.hpp"
#include "Game/Components/EnemyTag.hpp"

#include "Input/InputManager.hpp"

#include <Windows.h>
#include <cmath>

static float clampf(float v, float a, float b) { return (v < a) ? a : (v > b) ? b : v; }

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

  // Game rules (Step 11.1):
  // player colliding with walls or enemies => game over.
  if (m_ctx.world->has<WallTag>(other) || m_ctx.world->has<EnemyTag>(other))
  {
    m_ctx.app->requestGameOver();
  }
}

