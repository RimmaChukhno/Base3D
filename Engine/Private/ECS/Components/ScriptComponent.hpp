#pragma once

#include <memory>
#include <vector>

class Script;

// Move-only component that owns one-or-more script instances (Unity-like).
// We define destructor/move ops out-of-line to:
// - keep Script forward-declared here (avoid heavy includes in ECS headers)
// - guarantee OnDestroy is called exactly once when the component is removed
struct ScriptComponent
{
  struct Slot
  {
    std::unique_ptr<Script> script;
    bool started = false;

    Slot();
    explicit Slot(std::unique_ptr<Script> s);

    Slot(const Slot&) = delete;
    Slot& operator=(const Slot&) = delete;

    Slot(Slot&&) noexcept;
    Slot& operator=(Slot&&) noexcept;

    ~Slot();
  };

  std::vector<Slot> scripts;

  ScriptComponent();
  explicit ScriptComponent(std::unique_ptr<Script> s);
  void add(std::unique_ptr<Script> s);

  ScriptComponent(const ScriptComponent&) = delete;
  ScriptComponent& operator=(const ScriptComponent&) = delete;

  ScriptComponent(ScriptComponent&&) noexcept;
  ScriptComponent& operator=(ScriptComponent&&) noexcept;

  ~ScriptComponent();

private:
  void destroy();
};

