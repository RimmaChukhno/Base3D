#pragma once

#include <memory>

class Script;

// Move-only component that owns the script instance.
// We define destructor/move ops out-of-line to:
// - keep Script forward-declared here (avoid heavy includes in ECS headers)
// - guarantee OnDestroy is called exactly once when the component is removed
struct ScriptComponent
{
  std::unique_ptr<Script> script;
  bool started = false;

  ScriptComponent();
  explicit ScriptComponent(std::unique_ptr<Script> s);

  ScriptComponent(const ScriptComponent&) = delete;
  ScriptComponent& operator=(const ScriptComponent&) = delete;

  ScriptComponent(ScriptComponent&&) noexcept;
  ScriptComponent& operator=(ScriptComponent&&) noexcept;

  ~ScriptComponent();

private:
  void destroy();
};

