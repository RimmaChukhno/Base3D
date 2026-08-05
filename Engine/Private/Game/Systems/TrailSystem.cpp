#include "TrailSystem.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/MeshRenderer.hpp"

#include "Game/Components/TrailSegment.hpp"

#include "Time/TimeSystem.hpp"

#include <vector>

void TrailSystem::update(World& world, TimeSystem& time)
{
  auto* st = world.tryStorage<TrailSegment>();
  if (!st) return;

  const float dt = time.deltaSeconds();
  const auto& entities = st->denseEntities();
  auto& segs = st->denseComponents();

  std::vector<EntityId> toDestroy;
  toDestroy.reserve(256);

  for (uint32_t i = 0; i < st->size(); ++i)
  {
    TrailSegment& s = segs[i];
    s.ttl -= dt;

    // Optional fade-out (alpha)
    if (auto* mr = world.tryGet<MeshRenderer>(entities[i]))
    {
      const float t = (s.ttlMax > 1e-6f) ? (s.ttl / s.ttlMax) : 0.0f;
      const float a = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
      mr->tintA = a;
    }

    if (s.ttl <= 0.0f)
    {
      toDestroy.push_back(entities[i]);
    }
  }

  for (const EntityId e : toDestroy)
  {
    world.destroyEntity(e);
  }
}

