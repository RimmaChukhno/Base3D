#pragma once

#include "ECS/EntityId.hpp"

struct TrailSegment
{
  EntityId owner = kInvalidEntity;
  float ttl = 8.0f;
  float ttlMax = 8.0f;
  float spawnTime = 0.0f;
};

