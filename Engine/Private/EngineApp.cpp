#include "EngineApp.hpp"

#include "Renderer/D3D11Renderer.hpp"
#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"
#include "ECS/World.hpp"
#include "ECS/EntityId.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Velocity.hpp"
#include "ECS/Components/MeshRenderer.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "ECS/Components/Collider.hpp"
#include "ECS/Systems/CollisionSystem.hpp"

#include "ECS/Components/ScriptComponent.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "Scripting/Scripts/SpinScript.hpp"

#include "Application/StateMachine/StateMachine.hpp"
#include "Application/StateMachine/GameStateId.hpp"

#include "Particles/ParticleSystem.hpp"
#include "ECS/Components/ParticleEmitter.hpp"

#include <Windows.h>
#include <cmath>

EngineApp::EngineApp() = default;

EngineApp::~EngineApp()
{
  shutdown();
}

bool EngineApp::init(const EngineConfig& cfg)
{
  m_cfg = cfg;
  m_assetsRoot = cfg.assetsRoot ? cfg.assetsRoot : L"";
  m_cfg.assetsRoot = m_assetsRoot.c_str();

  if (m_renderer)
  {
    return true;
  }

  m_time = new TimeSystem();
  if (!m_time->init())
  {
    delete m_time;
    m_time = nullptr;
    return false;
  }

  m_input = new InputManager();
  if (!m_input->init())
  {
    delete m_input;
    m_input = nullptr;
    delete m_time;
    m_time = nullptr;
    return false;
  }

  m_renderer = new D3D11Renderer();
  if (!m_renderer->init(m_cfg))
  {
    delete m_renderer;
    m_renderer = nullptr;
    delete m_input;
    m_input = nullptr;
    delete m_time;
    m_time = nullptr;
    return false;
  }

  m_renderSystem = new RenderSystem();
  m_collisionSystem = new CollisionSystem();
  m_scriptSystem = new ScriptSystem();
  m_stateMachine = new StateMachine();
  m_particleSystem = new ParticleSystem();

  m_world = new World();
  m_testEntities.clear();
  m_testEntities.reserve(2000);

  // Step 5 validation:
  // - Spawn a lot of entities (ECS perf)
  // - Render a subset via MeshRenderer (multiple meshes/materials)
  constexpr int kSpawnCount = 2000;
  for (int i = 0; i < kSpawnCount; ++i)
  {
    const EntityId e = m_world->createEntity();
    Transform t{};
    t.px = (static_cast<float>(i % 50) - 25.0f) * 0.05f;
    t.py = (static_cast<float>(i / 50) - 20.0f) * 0.05f;
    t.pz = 0.0f;
    m_world->add<Transform>(e, t);

    if ((i % 3) == 0)
    {
      Velocity v{};
      v.vx = 0.1f + 0.02f * static_cast<float>(i % 7);
      v.vy = 0.05f;
      v.vz = 0.0f;
      m_world->add<Velocity>(e, v);
    }

    // Only some entities are renderable to keep the demo fast while proving the system.
    if ((i % 10) == 0)
    {
      MeshRenderer mr{};
      mr.material = m_renderer->defaultColorMaterial();
      mr.mesh = ((i % 20) == 0) ? m_renderer->defaultQuadMesh() : m_renderer->defaultTriangleMesh();
      mr.tintR = (i % 2) ? 1.0f : 0.4f;
      mr.tintG = (i % 3) ? 0.9f : 0.4f;
      mr.tintB = (i % 5) ? 0.8f : 0.4f;
      mr.tintA = 1.0f;
      m_world->add<MeshRenderer>(e, mr);

      // Step 6: give renderable entities an AABB collider.
      // Keep extents small so collisions happen with motion.
      m_world->add<Collider>(e, Collider::makeAabb(0.03f, 0.03f, 0.01f));

      // Step 7: attach a script to a subset.
      if ((i % 100) == 0)
      {
        m_world->add<ScriptComponent>(e, ScriptComponent{ std::make_unique<SpinScript>() });
      }
    }

    m_testEntities.push_back(e);
  }

  // Step 9: create a particle emitter entity (visible in Gameplay state).
  {
    const EntityId pe = m_world->createEntity();
    Transform t{};
    t.px = 0.0f;
    t.py = 0.0f;
    t.pz = 0.0f;
    m_world->add<Transform>(pe, t);

    ParticleEmitter em{};
    em.emitRate = 350.0f;
    em.maxParticles = 900;
    em.spawnRadius = 0.02f;
    em.lifeMin = 0.5f;
    em.lifeMax = 1.1f;
    em.speedMin = 0.10f;
    em.speedMax = 0.35f;
    em.sizeMin = 0.008f;
    em.sizeMax = 0.020f;
    m_world->add<ParticleEmitter>(pe, std::move(em));
  }

  // State machine (Step 8) comes last so it can rely on subsystems.
  m_stateMachine->init(*this, *m_input, *m_time);

  return true;
}

void EngineApp::tick()
{
  if (!m_renderer || !m_input || !m_time) return;

  m_time->tick();
  m_input->tick();

  // Example controls for validation:
  // - ESC closes the window (proves key pressed works).
  // - Mouse position affects triangle translation (proves mouse + resize path).
  if (m_input->wasKeyPressed(VK_ESCAPE))
  {
    PostMessageW(reinterpret_cast<HWND>(m_cfg.windowHandle), WM_CLOSE, 0, 0);
  }

  float nx = 0.0f;
  float ny = 0.0f;
  if (m_cfg.width > 0 && m_cfg.height > 0)
  {
    nx = (static_cast<float>(m_input->mouseX()) / static_cast<float>(m_cfg.width)) * 2.0f - 1.0f;
    ny = 1.0f - (static_cast<float>(m_input->mouseY()) / static_cast<float>(m_cfg.height)) * 2.0f;
  }

  // State machine decides which pipeline stages run this frame.
  FramePlan plan{};
  if (m_stateMachine)
  {
    plan = m_stateMachine->tick();
  }
  else
  {
    // Fallback: old behavior = gameplay pipeline always on.
    plan.runScripts = true;
    plan.runMotion = true;
    plan.runCollision = true;
    plan.runParticles = true;
    plan.runRender = true;
  }

  // Post-process tweak controls (works in any state).
  // Brightness: 1/2, Contrast: 3/4, Saturation: 5/6, Toggle post: O
  static float s_brightness = 0.0f;
  static float s_contrast = 1.0f;
  static float s_saturation = 1.0f;
  static bool s_postEnabled = true;

  const float step = m_input->isKeyDown(VK_SHIFT) ? 0.10f : 0.02f;
  if (m_input->wasKeyPressed('O')) s_postEnabled = !s_postEnabled;
  if (m_input->isKeyDown('1')) s_brightness -= step;
  if (m_input->isKeyDown('2')) s_brightness += step;
  if (m_input->isKeyDown('3')) s_contrast -= step;
  if (m_input->isKeyDown('4')) s_contrast += step;
  if (m_input->isKeyDown('5')) s_saturation -= step;
  if (m_input->isKeyDown('6')) s_saturation += step;

  if (s_contrast < 0.0f) s_contrast = 0.0f;
  if (s_saturation < 0.0f) s_saturation = 0.0f;
  if (s_contrast > 2.0f) s_contrast = 2.0f;
  if (s_saturation > 2.0f) s_saturation = 2.0f;
  if (s_brightness < -1.0f) s_brightness = -1.0f;
  if (s_brightness > 1.0f) s_brightness = 1.0f;

  m_renderer->setPostProcess(s_brightness, s_contrast, s_saturation);
  m_renderer->setPostEnabled(s_postEnabled);

  // Script update stage (Unity-like).
  if (plan.runScripts && m_world && m_scriptSystem)
  {
    m_scriptSystem->update(*m_world, *m_input, *m_time);
  }

  // Motion stage (Velocity integration) + simple global wobble (kept as a demo).
  if (plan.runMotion && m_world)
  {
    const float t = m_time->totalSeconds();
    const float wobble = 0.1f * std::sinf(t);

    int movingCount = 0;
    const float dt = m_time->deltaSeconds();
    m_world->each<Transform, Velocity>([&](EntityId, Transform& tr, Velocity& v)
    {
      tr.px += v.vx * dt;
      tr.py += v.vy * dt;
      tr.rz += dt * 0.5f;
      movingCount += 1;
    });

    auto& transforms = m_world->storage<Transform>();
    auto& comps = transforms.denseComponents();
    for (auto& tr : comps)
    {
      tr.px += wobble * dt;
    }

    m_frameStats.movingCount = movingCount;
  }

  // Update stats for external debugging (Game window title).
  m_frameStats.deltaSeconds = m_time->deltaSeconds();
  m_frameStats.totalSeconds = m_time->totalSeconds();
  m_frameStats.fps = m_time->fps();
  m_frameStats.entityCount = m_world ? static_cast<int32_t>(m_world->aliveCount()) : 0;
  // movingCount is computed above only when world exists; default to 0 otherwise.
  if (!m_world) m_frameStats.movingCount = 0;
  m_frameStats.drawCount = 0;
  m_frameStats.particleVertexCount = 0;
  m_frameStats.collisionPairs = 0;
  m_frameStats.collisionOverlaps = 0;
  m_frameStats.stateId = m_stateMachine ? static_cast<int32_t>(m_stateMachine->current()) : -1;
  m_frameStats.mouseX = m_input->mouseX();
  m_frameStats.mouseY = m_input->mouseY();
  m_frameStats.mouseDeltaX = m_input->mouseDeltaX();
  m_frameStats.mouseDeltaY = m_input->mouseDeltaY();
  m_frameStats.wheelDelta = m_input->wheelDelta();

  m_renderer->beginFrame();
  m_renderer->clear(0.05f, 0.10f, 0.20f, 1.0f);
  if (plan.runCollision && m_world && m_collisionSystem)
  {
    const auto cs = m_collisionSystem->update(*m_world, *m_renderer, m_cfg.width, m_cfg.height, &m_overlapPairs);
    m_frameStats.collisionPairs = cs.pairChecks;
    m_frameStats.collisionOverlaps = cs.overlaps;

    // Script callbacks for collisions.
    if (m_scriptSystem)
    {
      for (const auto& p : m_overlapPairs)
      {
        m_scriptSystem->onCollision(*m_world, p.first, p.second);
      }
    }
  }

  // Particles stage (update + render quads).
  if (plan.runParticles && m_world && m_particleSystem)
  {
    const uint32_t vtx = m_particleSystem->update(*m_world, *m_time);
    m_frameStats.particleVertexCount = static_cast<int32_t>(vtx);

    // Aspect-only MVP (consistent with our current 2D-ish rendering).
    const float aspect = (m_cfg.height > 0) ? (static_cast<float>(m_cfg.width) / static_cast<float>(m_cfg.height)) : 1.0f;
    const float sx = 1.0f / (aspect > 0.0001f ? aspect : 1.0f);
    const float mvp[16] = {
      sx, 0,  0,  0,
      0,  1,  0,  0,
      0,  0,  1,  0,
      0,  0,  0,  1
    };

    const auto& verts = m_particleSystem->vertices();
    if (!verts.empty())
    {
      m_renderer->drawParticles(verts.data(), sizeof(ParticleVertex), static_cast<uint32_t>(verts.size()), mvp);
    }
  }

  if (plan.runRender && m_world && m_renderSystem)
  {
    m_frameStats.drawCount = m_renderSystem->render(*m_world, *m_renderer, m_cfg.width, m_cfg.height);
  }
  m_renderer->endFrame();
  m_renderer->present();
}

void EngineApp::onResize(int32_t width, int32_t height)
{
  if (!m_renderer) return;
  m_cfg.width = width;
  m_cfg.height = height;
  m_renderer->resize(width, height);
}

void EngineApp::onWin32Message(uint32_t msg, uintptr_t wParam, intptr_t lParam)
{
  if (!m_input) return;
  m_input->onWin32Message(msg, wParam, lParam);
}

void EngineApp::requestQuit()
{
  if (!m_cfg.windowHandle) return;
  PostMessageW(reinterpret_cast<HWND>(m_cfg.windowHandle), WM_CLOSE, 0, 0);
}

void EngineApp::getFrameStats(EngineFrameStats& outStats) const
{
  outStats = m_frameStats;
}

void EngineApp::shutdown()
{
  if (m_renderer)
  {
    m_renderer->shutdown();
    delete m_renderer;
    m_renderer = nullptr;
  }

  delete m_renderSystem;
  m_renderSystem = nullptr;

  delete m_collisionSystem;
  m_collisionSystem = nullptr;

  delete m_scriptSystem;
  m_scriptSystem = nullptr;

  delete m_stateMachine;
  m_stateMachine = nullptr;

  delete m_particleSystem;
  m_particleSystem = nullptr;

  delete m_world;
  m_world = nullptr;
  m_testEntities.clear();

  delete m_input;
  m_input = nullptr;

  delete m_time;
  m_time = nullptr;
}

