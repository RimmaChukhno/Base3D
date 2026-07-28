#pragma once

#include "ECS/EntityId.hpp"

// Type-erased base for component storages so World can:
// - remove components when an entity is destroyed
// - keep heterogeneous storages in one container
class IEcsStorage
{
public:
  virtual ~IEcsStorage() = default;
  virtual void onEntityDestroyed(EntityId e) = 0;
};

