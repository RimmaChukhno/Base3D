#pragma once

#include "EngineAPI.hpp"
#include "Renderer/Resources/Handles.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class D3D11Renderer;
class InputManager;
class TimeSystem;
class World;
struct EntityId;
class RenderSystem;
class CollisionSystem;
class ScriptSystem;
class StateMachine;
class ParticleSystem;
class ResourceManager;

class EngineApp
{
public:
  EngineApp();
  ~EngineApp();

  EngineApp(const EngineApp&) = delete;
  EngineApp& operator=(const EngineApp&) = delete;

  bool init(const EngineConfig& cfg);
  void tick();
  void onResize(int32_t width, int32_t height);
  void onWin32Message(uint32_t msg, uintptr_t wParam, intptr_t lParam);
  void requestQuit();
  void getFrameStats(EngineFrameStats& outStats) const;
  void shutdown();

private:
  EngineConfig m_cfg{};
  std::wstring m_assetsRoot;
  EngineFrameStats m_frameStats{};
  D3D11Renderer* m_renderer = nullptr;
  InputManager* m_input = nullptr;
  TimeSystem* m_time = nullptr;

  // ECS (Step 4)
  World* m_world = nullptr;
  std::vector<EntityId> m_testEntities;

  RenderSystem* m_renderSystem = nullptr;
  CollisionSystem* m_collisionSystem = nullptr;
  ScriptSystem* m_scriptSystem = nullptr;
  StateMachine* m_stateMachine = nullptr;

  // Particles (Step 9)
  ParticleSystem* m_particleSystem = nullptr;

  // Resources (Step 10)
  ResourceManager* m_resources = nullptr;
  MeshHandle m_triMesh = kInvalidMesh;
  MeshHandle m_quadMesh = kInvalidMesh;
  MaterialHandle m_meshColorMat = kInvalidMaterial;
  MaterialHandle m_particleMat = kInvalidMaterial;
  uint32_t m_particleTex = 0xFFFFFFFFu; // TextureHandle (kept uint32_t here to avoid public include)

  // Re-used buffer to avoid per-frame allocations.
  std::vector<std::pair<EntityId, EntityId>> m_overlapPairs;
};

