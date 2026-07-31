#pragma once

#include "Scripting/ScriptContext.hpp"

// Unity-like lifecycle (minimal):
// - Start(): called once before the first Update()
// - Update(dt): called every frame
// - OnCollision(other): called when CollisionSystem reports an overlap
// - OnDestroy(): called when ScriptComponent is removed/destroyed
class Script
{
public:
  virtual ~Script() = default;

  void setContext(const ScriptContext& ctx) { m_ctx = ctx; }
  const ScriptContext& context() const { return m_ctx; }

  virtual void Start() {}
  virtual void Update(float /*dt*/) {}
  virtual void OnCollision(EntityId /*other*/) {}
  virtual void OnDestroy() {}

protected:
  ScriptContext m_ctx{};
};

