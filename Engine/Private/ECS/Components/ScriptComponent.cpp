#include "ScriptComponent.hpp"

#include "Scripting/Script.hpp"

ScriptComponent::Slot::Slot() = default;

ScriptComponent::Slot::Slot(std::unique_ptr<Script> s)
  : script(std::move(s))
{
}

ScriptComponent::Slot::Slot(Slot&& other) noexcept
  : script(std::move(other.script))
  , started(other.started)
{
  other.started = false;
}

ScriptComponent::Slot& ScriptComponent::Slot::operator=(Slot&& other) noexcept
{
  if (this == &other) return *this;
  if (script)
  {
    script->OnDestroy();
    script.reset();
  }
  script = std::move(other.script);
  started = other.started;
  other.started = false;
  return *this;
}

ScriptComponent::Slot::~Slot()
{
  if (script)
  {
    script->OnDestroy();
    script.reset();
  }
  started = false;
}

ScriptComponent::ScriptComponent() = default;

ScriptComponent::ScriptComponent(std::unique_ptr<Script> s)
{
  add(std::move(s));
}

ScriptComponent::ScriptComponent(ScriptComponent&& other) noexcept
  : scripts(std::move(other.scripts))
{
}

ScriptComponent& ScriptComponent::operator=(ScriptComponent&& other) noexcept
{
  if (this == &other) return *this;
  destroy();
  scripts = std::move(other.scripts);
  return *this;
}

ScriptComponent::~ScriptComponent()
{
  destroy();
}

void ScriptComponent::add(std::unique_ptr<Script> s)
{
  if (!s) return;
  scripts.emplace_back(std::move(s));
}

void ScriptComponent::destroy()
{
  for (auto& slot : scripts)
  {
    if (slot.script)
    {
      slot.script->OnDestroy();
      slot.script.reset();
    }
    slot.started = false;
  }
  scripts.clear();
}

