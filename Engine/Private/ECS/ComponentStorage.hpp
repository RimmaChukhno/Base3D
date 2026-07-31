#pragma once

#include "ECS/EntityId.hpp"

#include <cstdint>
#include <algorithm>
#include <utility>
#include <vector>

// Sparse-set storage (Unity/EnTT style idea):
// - denseEntities[i] and denseComponents[i] are tightly packed
// - sparse[index] maps entity.index -> denseIndex+1 (0 means "not present")
//
// This gives:
// - O(1) add/remove/has/get (amortized)
// - tight iteration over components (cache-friendly)
template <typename T>
class ComponentStorage
{
public:
  void reserve(uint32_t n)
  {
    m_denseEntities.reserve(n);
    m_denseComponents.reserve(n);
  }

  void ensureSparseCapacity(uint32_t entityCapacity)
  {
    if (m_sparse.size() < entityCapacity)
    {
      m_sparse.resize(entityCapacity, 0);
    }
  }

  bool has(EntityId e) const
  {
    if (e.index >= m_sparse.size()) return false;
    const uint32_t slot = m_sparse[e.index];
    if (slot == 0) return false;
    const uint32_t denseIndex = slot - 1;
    return denseIndex < m_denseEntities.size() && m_denseEntities[denseIndex] == e;
  }

  T* tryGet(EntityId e)
  {
    if (!has(e)) return nullptr;
    const uint32_t denseIndex = m_sparse[e.index] - 1;
    return &m_denseComponents[denseIndex];
  }

  const T* tryGet(EntityId e) const
  {
    if (!has(e)) return nullptr;
    const uint32_t denseIndex = m_sparse[e.index] - 1;
    return &m_denseComponents[denseIndex];
  }

  // Add/replace component.
  // Takes by-value to support move-only types (e.g., ScriptComponent with unique_ptr).
  T& add(EntityId e, T value = T{})
  {
    if (has(e))
    {
      // Replace-in-place semantics (simple and deterministic).
      T* existing = tryGet(e);
      *existing = std::move(value);
      return *existing;
    }

    ensureSparseCapacity(e.index + 1);

    const uint32_t denseIndex = static_cast<uint32_t>(m_denseEntities.size());
    m_denseEntities.push_back(e);
    m_denseComponents.push_back(std::move(value));
    m_sparse[e.index] = denseIndex + 1;
    return m_denseComponents.back();
  }

  template <typename... Args>
  T& emplace(EntityId e, Args&&... args)
  {
    return add(e, T{ std::forward<Args>(args)... });
  }

  void remove(EntityId e)
  {
    if (!has(e)) return;

    const uint32_t denseIndex = m_sparse[e.index] - 1;
    const uint32_t last = static_cast<uint32_t>(m_denseEntities.size() - 1);

    if (denseIndex != last)
    {
      // Swap-remove.
      m_denseEntities[denseIndex] = m_denseEntities[last];
      m_denseComponents[denseIndex] = std::move(m_denseComponents[last]);
      m_sparse[m_denseEntities[denseIndex].index] = denseIndex + 1;
    }

    m_denseEntities.pop_back();
    m_denseComponents.pop_back();
    m_sparse[e.index] = 0;
  }

  void clear()
  {
    m_denseEntities.clear();
    m_denseComponents.clear();
    std::fill(m_sparse.begin(), m_sparse.end(), 0);
  }

  uint32_t size() const { return static_cast<uint32_t>(m_denseEntities.size()); }

  // Iteration API (simple for systems).
  // NOTE: We deliberately expose dense arrays to allow cache-friendly loops.
  const std::vector<EntityId>& denseEntities() const { return m_denseEntities; }
  std::vector<T>& denseComponents() { return m_denseComponents; }
  const std::vector<T>& denseComponents() const { return m_denseComponents; }

private:
  std::vector<uint32_t> m_sparse;          // entity.index -> denseIndex+1
  std::vector<EntityId> m_denseEntities;   // denseIndex -> entity
  std::vector<T> m_denseComponents;        // denseIndex -> component
};

