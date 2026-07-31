#include "SpinScript.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/MeshRenderer.hpp"

void SpinScript::Start()
{
  // Make it visually obvious that Start() ran once.
  if (!m_ctx.world) return;
  if (auto* mr = m_ctx.world->tryGet<MeshRenderer>(m_ctx.self))
  {
    mr->tintR = 0.6f;
    mr->tintG = 1.0f;
    mr->tintB = 0.6f;
  }
}

void SpinScript::Update(float dt)
{
  if (!m_ctx.world) return;
  if (auto* tr = m_ctx.world->tryGet<Transform>(m_ctx.self))
  {
    tr->rz += dt * m_speed;
  }
}

void SpinScript::OnCollision(EntityId /*other*/)
{
  // Flash red when colliding (in addition to CollisionSystem tinting).
  if (!m_ctx.world) return;
  if (auto* mr = m_ctx.world->tryGet<MeshRenderer>(m_ctx.self))
  {
    mr->tintR = 1.0f;
    mr->tintG = 0.2f;
    mr->tintB = 0.2f;
  }
}

