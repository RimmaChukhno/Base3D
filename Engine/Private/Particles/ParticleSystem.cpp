#include "ParticleSystem.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/ParticleEmitter.hpp"
#include "ECS/Components/Transform.hpp"

#include "Time/TimeSystem.hpp"

#include <algorithm>
#include <cmath>

static float lerp(float a, float b, float t) { return a + (b - a) * t; }

float ParticleSystem::rand01()
{
  // xorshift32
  uint32_t x = m_rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  m_rng = x;
  return static_cast<float>(x) / static_cast<float>(0xFFFFFFFFu);
}

uint32_t ParticleSystem::update(World& world, TimeSystem& time)
{
  auto* emitters = world.tryStorage<ParticleEmitter>();
  if (!emitters) return 0;

  const float dt = time.deltaSeconds();
  const auto& entities = emitters->denseEntities();
  auto& comps = emitters->denseComponents();

  // Build vertices anew each frame (simple; can be optimized later).
  m_vertices.clear();
  m_vertices.reserve(12000);

  for (uint32_t i = 0; i < emitters->size(); ++i)
  {
    const EntityId e = entities[i];
    ParticleEmitter& em = comps[i];

    // Ensure capacity.
    if (em.particles.capacity() < em.maxParticles)
    {
      em.particles.reserve(em.maxParticles);
    }

    const Transform* tr = world.tryGet<Transform>(e);
    const float baseX = tr ? tr->px : 0.0f;
    const float baseY = tr ? tr->py : 0.0f;
    const float baseZ = tr ? tr->pz : 0.0f;

    // Emit
    em.emitAccum += dt * em.emitRate;
    int toSpawn = static_cast<int>(em.emitAccum);
    if (toSpawn > 0) em.emitAccum -= static_cast<float>(toSpawn);

    while (toSpawn-- > 0 && em.particles.size() < em.maxParticles)
    {
      const float angle = rand01() * 6.2831853f;
      const float rad = rand01() * em.spawnRadius;
      const float ox = std::cos(angle) * rad;
      const float oy = std::sin(angle) * rad;

      const float speed = lerp(em.speedMin, em.speedMax, rand01());
      const float vx = std::cos(angle) * speed;
      const float vy = std::sin(angle) * speed;

      Particle p{};
      p.px = baseX + ox;
      p.py = baseY + oy;
      p.pz = baseZ;
      p.vx = vx;
      p.vy = vy;
      p.vz = 0.0f;
      p.lifeMax = lerp(em.lifeMin, em.lifeMax, rand01());
      p.life = p.lifeMax;
      p.size = lerp(em.sizeMin, em.sizeMax, rand01());
      p.rotation = rand01() * 6.2831853f;
      p.r = 1.0f;
      p.g = 0.8f;
      p.b = 0.3f;
      p.a = 1.0f;

      em.particles.push_back(p);
    }

    // Update & remove dead (swap-remove)
    for (size_t k = 0; k < em.particles.size();)
    {
      Particle& p = em.particles[k];
      p.life -= dt;
      if (p.life <= 0.0f)
      {
        em.particles[k] = em.particles.back();
        em.particles.pop_back();
        continue;
      }

      p.px += p.vx * dt;
      p.py += p.vy * dt;

      // Fade out
      const float t = std::clamp(p.life / p.lifeMax, 0.0f, 1.0f);
      p.a = t;

      k++;
    }

    // Generate quads (6 verts/particle)
    for (const Particle& p : em.particles)
    {
      const float s = p.size;

      const float x0 = p.px - s;
      const float x1 = p.px + s;
      const float y0 = p.py - s;
      const float y1 = p.py + s;
      const float z = p.pz;

      const ParticleVertex v0{ x0, y0, z, 0.0f, 1.0f, p.r, p.g, p.b, p.a };
      const ParticleVertex v1{ x0, y1, z, 0.0f, 0.0f, p.r, p.g, p.b, p.a };
      const ParticleVertex v2{ x1, y1, z, 1.0f, 0.0f, p.r, p.g, p.b, p.a };
      const ParticleVertex v3{ x1, y0, z, 1.0f, 1.0f, p.r, p.g, p.b, p.a };

      // tri 1: 0-1-2
      m_vertices.push_back(v0);
      m_vertices.push_back(v1);
      m_vertices.push_back(v2);
      // tri 2: 0-2-3
      m_vertices.push_back(v0);
      m_vertices.push_back(v2);
      m_vertices.push_back(v3);
    }
  }

  return static_cast<uint32_t>(m_vertices.size());
}

