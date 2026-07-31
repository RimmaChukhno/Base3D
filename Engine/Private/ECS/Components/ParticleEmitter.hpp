#pragma once

#include <cstdint>
#include <vector>

struct Particle
{
  float px = 0.0f;
  float py = 0.0f;
  float pz = 0.0f;

  float vx = 0.0f;
  float vy = 0.0f;
  float vz = 0.0f;

  float life = 0.0f;
  float lifeMax = 1.0f;

  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;

  float size = 0.02f;
  float rotation = 0.0f;
};

struct ParticleEmitter
{
  // Config
  float emitRate = 200.0f;    // particles/sec
  float lifeMin = 0.6f;
  float lifeMax = 1.2f;
  float speedMin = 0.05f;
  float speedMax = 0.25f;
  float sizeMin = 0.01f;
  float sizeMax = 0.03f;

  // Spawn shape (local space)
  float spawnRadius = 0.02f;

  // Runtime
  float emitAccum = 0.0f;
  uint32_t maxParticles = 800;
  std::vector<Particle> particles;
};

