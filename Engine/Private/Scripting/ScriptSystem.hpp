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

private:
  // Cached pointers from the last update() call so OnCollision
  // can provide a meaningful ScriptContext.
  EngineApp* m_app = nullptr;
  InputManager* m_input = nullptr;
  TimeSystem* m_time = nullptr;
};

