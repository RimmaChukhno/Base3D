#pragma once

#include "ECS/EntityId.hpp"

class World;
class ResourceManager;

struct GameWorldRefs
{
  EntityId player = kInvalidEntity;
  EntityId scoreEntity = kInvalidEntity;
  EntityId enemy = kInvalidEntity;
};

class GameWorld
{
public:
  GameWorldRefs build(World& world, ResourceManager& resources);
};

