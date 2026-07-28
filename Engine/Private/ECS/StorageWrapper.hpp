#pragma once

#include "ECS/ComponentStorage.hpp"
#include "ECS/IStorage.hpp"

template <typename T>
class StorageWrapper final : public IEcsStorage
{
public:
  ComponentStorage<T> storage;

  void onEntityDestroyed(EntityId e) override
  {
    storage.remove(e);
  }
};

