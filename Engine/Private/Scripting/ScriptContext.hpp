#pragma once

#include "ECS/EntityId.hpp"

class World;
class InputManager;
class TimeSystem;

struct ScriptContext
{
  World* world = nullptr;
  EntityId self{};
  InputManager* input = nullptr;
  TimeSystem* time = nullptr;
};

