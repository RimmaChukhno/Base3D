#pragma once

#include "ECS/EntityRegistry.hpp"
#include "ECS/IStorage.hpp"
#include "ECS/StorageWrapper.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>

// World ties together:
// - EntityRegistry (lifetimes)
// - Component storages (data)
//
// This "registry + storages" layout is a proven foundation.
// We can later add:
// - queries/views (multi-component iteration)
// - system scheduling
// - events
class World
{
public:
  EntityId createEntity()
  {
    const EntityId e = m_entities.create();
    return e;
  }

  void destroyEntity(EntityId e)
  {
    if (!m_entities.isAlive(e)) return;

    for (auto& [_, storage] : m_storages)
    {
      storage->onEntityDestroyed(e);
    }

    m_entities.destroy(e);
  }

  bool isAlive(EntityId e) const { return m_entities.isAlive(e); }
  uint32_t aliveCount() const { return m_entities.aliveCount(); }
  uint32_t capacity() const { return m_entities.capacity(); }

  template <typename T>
  ComponentStorage<T>& storage()
  {
    const std::type_index key = std::type_index(typeid(T));
    auto it = m_storages.find(key);
    if (it == m_storages.end())
    {
      auto wrapper = std::make_unique<StorageWrapper<T>>();
      // Optional: reserve some space for student projects; can be tuned later.
      wrapper->storage.ensureSparseCapacity(m_entities.capacity());
      it = m_storages.emplace(key, std::move(wrapper)).first;
    }

    auto* typed = static_cast<StorageWrapper<T>*>(it->second.get());
    return typed->storage;
  }

  template <typename T>
  ComponentStorage<T>* tryStorage()
  {
    const std::type_index key = std::type_index(typeid(T));
    auto it = m_storages.find(key);
    if (it == m_storages.end()) return nullptr;
    auto* typed = static_cast<StorageWrapper<T>*>(it->second.get());
    return &typed->storage;
  }

  template <typename T>
  const ComponentStorage<T>* tryStorage() const
  {
    const std::type_index key = std::type_index(typeid(T));
    auto it = m_storages.find(key);
    if (it == m_storages.end()) return nullptr;
    auto* typed = static_cast<const StorageWrapper<T>*>(it->second.get());
    return &typed->storage;
  }

  template <typename T>
  bool has(EntityId e) const
  {
    const std::type_index key = std::type_index(typeid(T));
    auto it = m_storages.find(key);
    if (it == m_storages.end()) return false;
    auto* typed = static_cast<const StorageWrapper<T>*>(it->second.get());
    return typed->storage.has(e);
  }

  template <typename T>
  T& add(EntityId e, const T& value = T{})
  {
    // Safety: in a "pro" engine you'd likely assert here.
    // We keep it non-throwing and tolerant for now.
    if (!m_entities.isAlive(e))
    {
      // Create on invalid is a bug; return a stable reference anyway by creating a dummy entity.
      // For Step 4 we just avoid UB.
      e = createEntity();
    }

    auto& st = storage<T>();
    st.ensureSparseCapacity(m_entities.capacity());
    return st.add(e, value);
  }

  template <typename T>
  T* tryGet(EntityId e)
  {
    auto& st = storage<T>();
    return st.tryGet(e);
  }

  // ------------------------------------------------------------
  // Views / Queries (Option A)
  //
  // We start with a pragmatic, professional approach used by many engines:
  // - iterate the smallest dense storage
  // - for each entity, probe other components with O(1) sparse-set lookup
  //
  // This gives excellent real-world performance while staying simple.
  // ------------------------------------------------------------

  template <typename A, typename B, typename Fn>
  void each(Fn&& fn)
  {
    auto* sa = tryStorage<A>();
    auto* sb = tryStorage<B>();
    if (!sa || !sb) return;

    // Iterate the smaller dense array for better cache behavior.
    if (sa->size() <= sb->size())
    {
      const auto& entities = sa->denseEntities();
      auto& compsA = sa->denseComponents();
      for (uint32_t i = 0; i < sa->size(); ++i)
      {
        const EntityId e = entities[i];
        B* b = sb->tryGet(e);
        if (!b) continue;
        fn(e, compsA[i], *b);
      }
    }
    else
    {
      const auto& entities = sb->denseEntities();
      auto& compsB = sb->denseComponents();
      for (uint32_t i = 0; i < sb->size(); ++i)
      {
        const EntityId e = entities[i];
        A* a = sa->tryGet(e);
        if (!a) continue;
        fn(e, *a, compsB[i]);
      }
    }
  }

private:
  EntityRegistry m_entities;
  std::unordered_map<std::type_index, std::unique_ptr<IEcsStorage>> m_storages;
};

