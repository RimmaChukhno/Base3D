#pragma once

#include <cstdint>
#include <vector>

class World;
class TimeSystem;

struct ParticleVertex
{
  float px, py, pz;
  float u, v;
  float r, g, b, a;
};

class ParticleSystem
{
public:
  // Updates emitters and produces a flat vertex list for rendering.
  // Returns vertex count (multiple of 6).
  uint32_t update(World& world, TimeSystem& time);

  const std::vector<ParticleVertex>& vertices() const { return m_vertices; }

private:
  float rand01();

private:
  uint32_t m_rng = 0x12345678u;
  std::vector<ParticleVertex> m_vertices;
};

