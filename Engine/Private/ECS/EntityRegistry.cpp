#include "EntityRegistry.hpp"

EntityId EntityRegistry::create()
{
  uint32_t index = 0;

  if (!m_freeList.empty())
  {
    index = m_freeList.back();
    m_freeList.pop_back();
  }
  else
  {
    index = static_cast<uint32_t>(m_generations.size());
    m_generations.push_back(1); // generation starts at 1 (0 can represent "null" in some contexts)
  }

  m_aliveCount += 1;

  return EntityId{ index, m_generations[index] };
}

void EntityRegistry::destroy(EntityId e)
{
  if (!isAlive(e)) return;

  // Bump generation to invalidate stale handles.
  m_generations[e.index] += 1;

  // Recycle the index.
  m_freeList.push_back(e.index);

  m_aliveCount -= 1;
}

bool EntityRegistry::isAlive(EntityId e) const
{
  if (e.index >= m_generations.size()) return false;
  return m_generations[e.index] == e.generation;
}

