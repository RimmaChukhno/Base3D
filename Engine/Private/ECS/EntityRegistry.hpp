#pragma once

#include "ECS/EntityId.hpp"

#include <cstdint>
#include <vector>

// EntityRegistry owns entity lifetimes (create/destroy) and validity checks.
// It does NOT store components; World/ComponentStorage do that.
class EntityRegistry
{
public:
  EntityId create();
  void destroy(EntityId e);

  bool isAlive(EntityId e) const;

  uint32_t aliveCount() const { return m_aliveCount; }
  uint32_t capacity() const { return static_cast<uint32_t>(m_generations.size()); }

private:
  std::vector<uint32_t> m_generations;
  std::vector<uint32_t> m_freeList;
  uint32_t m_aliveCount = 0;
};

