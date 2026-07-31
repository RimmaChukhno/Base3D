#include "ScriptComponent.hpp"

#include "Scripting/Script.hpp"

ScriptComponent::ScriptComponent() = default;

ScriptComponent::ScriptComponent(std::unique_ptr<Script> s)
  : script(std::move(s))
{
}

ScriptComponent::ScriptComponent(ScriptComponent&& other) noexcept
  : script(std::move(other.script))
  , started(other.started)
{
  other.started = false;
}

ScriptComponent& ScriptComponent::operator=(ScriptComponent&& other) noexcept
{
  if (this == &other) return *this;
  destroy();
  script = std::move(other.script);
  started = other.started;
  other.started = false;
  return *this;
}

ScriptComponent::~ScriptComponent()
{
  destroy();
}

void ScriptComponent::destroy()
{
  if (script)
  {
    script->OnDestroy();
    script.reset();
  }
  started = false;
}

