#pragma once

#include "ECS/EntityId.hpp"

class EngineApp;
class World;
class InputManager;
class TimeSystem;

class ScriptSystem
{
public:
  void update(World& world, EngineApp& app, InputManager& input, TimeSystem& time);
  void onCollision(World& world, EntityId a, EntityId b);
};

