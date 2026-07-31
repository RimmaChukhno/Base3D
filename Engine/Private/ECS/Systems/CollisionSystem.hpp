#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "Renderer/Resources/Handles.hpp"

class World;
class D3D11Renderer;
struct EntityId;

struct CollisionStats
{
  int32_t colliderCount = 0;
  int32_t pairChecks = 0;
  int32_t overlaps = 0;
  int32_t debugLineVertexCount = 0;
};

class CollisionSystem
{
public:
  // Step 6: naive collision detection + debug draw.
  // If outOverlaps != nullptr, the system fills it with overlapping pairs for scripting callbacks.
  CollisionStats update(World& world, D3D11Renderer& renderer, int32_t viewportWidth, int32_t viewportHeight,
                        std::vector<std::pair<EntityId, EntityId>>* outOverlaps,
                        MaterialHandle debugMaterial);
};

