#include "ScriptSystem.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/ScriptComponent.hpp"

#include "Scripting/Script.hpp"
#include "Scripting/ScriptContext.hpp"

#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"

void ScriptSystem::update(World& world, InputManager& input, TimeSystem& time)
{
  auto* st = world.tryStorage<ScriptComponent>();
  if (!st) return;

  const auto& entities = st->denseEntities();
  auto& comps = st->denseComponents();

  const float dt = time.deltaSeconds();

  for (uint32_t i = 0; i < st->size(); ++i)
  {
    const EntityId e = entities[i];
    ScriptComponent& sc = comps[i];
    if (!sc.script) continue;

    sc.script->setContext(ScriptContext{ &world, e, &input, &time });

    if (!sc.started)
    {
      sc.script->Start();
      sc.started = true;
    }

    sc.script->Update(dt);
  }
}

void ScriptSystem::onCollision(World& world, EntityId a, EntityId b)
{
  auto* sa = world.tryGet<ScriptComponent>(a);
  if (sa && sa->script && sa->started)
  {
    sa->script->setContext(ScriptContext{ &world, a, nullptr, nullptr });
    sa->script->OnCollision(b);
  }

  auto* sb = world.tryGet<ScriptComponent>(b);
  if (sb && sb->script && sb->started)
  {
    sb->script->setContext(ScriptContext{ &world, b, nullptr, nullptr });
    sb->script->OnCollision(a);
  }
}

