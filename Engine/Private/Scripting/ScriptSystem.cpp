#include "ScriptSystem.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/ScriptComponent.hpp"

#include "Scripting/Script.hpp"
#include "Scripting/ScriptContext.hpp"

#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"
#include "EngineApp.hpp"

void ScriptSystem::update(World& world, EngineApp& app, InputManager& input, TimeSystem& time)
{
  m_app = &app;
  m_input = &input;
  m_time = &time;

  auto* st = world.tryStorage<ScriptComponent>();
  if (!st) return;

  const auto& entities = st->denseEntities();
  auto& comps = st->denseComponents();

  const float dt = time.deltaSeconds();

  for (uint32_t i = 0; i < st->size(); ++i)
  {
    const EntityId e = entities[i];
    ScriptComponent& sc = comps[i];
    if (sc.scripts.empty()) continue;

    for (auto& slot : sc.scripts)
    {
      if (!slot.script) continue;
      slot.script->setContext(ScriptContext{ &world, e, &app, &input, &time });

      if (!slot.started)
      {
        slot.script->Start();
        slot.started = true;
      }

      slot.script->Update(dt);
    }
  }
}

void ScriptSystem::onCollision(World& world, EntityId a, EntityId b)
{
  auto* sa = world.tryGet<ScriptComponent>(a);
  if (sa && !sa->scripts.empty())
  {
    for (auto& slot : sa->scripts)
    {
      if (!slot.script || !slot.started) continue;
      slot.script->setContext(ScriptContext{ &world, a, m_app, m_input, m_time });
      slot.script->OnCollision(b);
    }
  }

  auto* sb = world.tryGet<ScriptComponent>(b);
  if (sb && !sb->scripts.empty())
  {
    for (auto& slot : sb->scripts)
    {
      if (!slot.script || !slot.started) continue;
      slot.script->setContext(ScriptContext{ &world, b, m_app, m_input, m_time });
      slot.script->OnCollision(a);
    }
  }
}

