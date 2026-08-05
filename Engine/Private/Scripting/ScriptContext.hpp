#pragma once

#include "ECS/EntityId.hpp"

class World;
class InputManager;
class TimeSystem;
class EngineApp;

struct ScriptContext
{
  World* world = nullptr;
  EntityId self{};
  EngineApp* app = nullptr;
  InputManager* input = nullptr;
  TimeSystem* time = nullptr;
};

