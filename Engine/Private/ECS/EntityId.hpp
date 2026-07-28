#pragma once

#include <cstdint>

// EntityId is a stable handle: (index + generation).
// - index: slot in the registry arrays
// - generation: increments on destroy to invalidate stale handles
struct EntityId
{
  uint32_t index = 0;
  uint32_t generation = 0;
};

// For convenience in tests and containers.
inline constexpr EntityId kInvalidEntity{ 0xFFFFFFFFu, 0 };

inline bool operator==(const EntityId& a, const EntityId& b)
{
  return a.index == b.index && a.generation == b.generation;
}

inline bool operator!=(const EntityId& a, const EntityId& b)
{
  return !(a == b);
}

