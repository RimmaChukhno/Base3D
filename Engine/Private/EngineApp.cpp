#include "EngineApp.hpp"

#include "Renderer/D3D11Renderer.hpp"
#include "Input/InputManager.hpp"
#include "Time/TimeSystem.hpp"
#include "ECS/World.hpp"
#include "ECS/EntityId.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/MeshRenderer.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "ECS/Components/Collider.hpp"
#include "ECS/Systems/CollisionSystem.hpp"

#include "ECS/Components/ScriptComponent.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "Application/StateMachine/StateMachine.hpp"
#include "Application/StateMachine/GameStateId.hpp"

#include "Particles/ParticleSystem.hpp"
#include "ECS/Components/ParticleEmitter.hpp"

#include "Resources/ResourceManager.hpp"

#include "Game/GameWorld.hpp"
#include "Game/Components/ScoreComponent.hpp"
#include "Game/Components/PlayerTag.hpp"
#include "Game/Components/EnemyTag.hpp"
#include "Game/Components/WallTag.hpp"
#include "Game/Components/TrailTag.hpp"
#include "Game/Components/TrailSegment.hpp"
#include "Game/Systems/TrailSystem.hpp"

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
  m_resources = new ResourceManager(m_renderer->device(), *m_renderer, m_assetsRoot);
  m_trailSystem = new TrailSystem();

  // Built-in resources (dedup, Step 10).
  m_triMesh = m_resources->getOrCreateTriangleMesh();
  m_quadMesh = m_resources->getOrCreateQuadMesh();
  m_meshColorMat = m_resources->getOrCreateMeshColorMaterial();
  m_particleMat = m_resources->getOrCreateParticleMaterial();
  m_particleTex = m_resources->getOrCreateWhiteTexture1x1();

  // World is created by LoadingState via prepareGameplayWorld().
  m_world = nullptr;
  m_player = kInvalidEntity;
  m_scoreEntity = kInvalidEntity;

  // State machine (Step 8) comes last so it can rely on subsystems.
  m_stateMachine->init(*this, *m_input, *m_time);

  return true;
}

void EngineApp::tick()
{
  if (!m_renderer || !m_input || !m_time) return;

  m_time->tick();
  m_input->tick();

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
    m_scriptSystem->update(*m_world, *this, *m_input, *m_time);
  }

  // Motion stage is now script-driven for Step 11 (player/enemy scripts).
  if (!plan.runMotion) m_frameStats.movingCount = 0;

  // Trails TTL cleanup (Step 11.2)
  if (plan.runMotion && m_world && m_trailSystem)
  {
    m_trailSystem->update(*m_world, *m_time);
  }

  // Update stats for external debugging (Game window title).
  m_frameStats.deltaSeconds = m_time->deltaSeconds();
  m_frameStats.totalSeconds = m_time->totalSeconds();
  m_frameStats.fps = m_time->fps();
  m_frameStats.entityCount = m_world ? static_cast<int32_t>(m_world->aliveCount()) : 0;
  m_frameStats.drawCount = 0;
  m_frameStats.particleVertexCount = 0;
  m_frameStats.collisionPairs = 0;
  m_frameStats.collisionOverlaps = 0;
  m_frameStats.stateId = m_stateMachine ? static_cast<int32_t>(m_stateMachine->current()) : -1;
  m_frameStats.resMeshes = m_resources ? static_cast<int32_t>(m_resources->meshCount()) : 0;
  m_frameStats.resMaterials = m_resources ? static_cast<int32_t>(m_resources->materialCount()) : 0;
  m_frameStats.resShaders = m_resources ? static_cast<int32_t>(m_resources->shaderProgramCount()) : 0;
  m_frameStats.resTextures = m_resources ? static_cast<int32_t>(m_resources->textureCount()) : 0;
  m_frameStats.gameScore = 0;
  m_frameStats.gameTimeAlive = 0.0f;
  if (m_world && m_scoreEntity != kInvalidEntity)
  {
    if (auto* s = m_world->tryGet<ScoreComponent>(m_scoreEntity))
    {
      m_frameStats.gameScore = s->score;
      m_frameStats.gameTimeAlive = s->timeAlive;
    }
  }
  m_frameStats.mouseX = m_input->mouseX();
  m_frameStats.mouseY = m_input->mouseY();
  m_frameStats.mouseDeltaX = m_input->mouseDeltaX();
  m_frameStats.mouseDeltaY = m_input->mouseDeltaY();
  m_frameStats.wheelDelta = m_input->wheelDelta();

  m_renderer->beginFrame();
  m_renderer->clear(0.05f, 0.10f, 0.20f, 1.0f);
  if (plan.runCollision && m_world && m_collisionSystem)
  {
    const auto cs = m_collisionSystem->update(*m_world, *m_renderer, m_cfg.width, m_cfg.height, &m_overlapPairs, m_meshColorMat);
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
      ID3D11ShaderResourceView* particleSrv = (m_resources) ? m_resources->srv(m_particleTex) : nullptr;
      m_renderer->drawParticles(verts.data(), sizeof(ParticleVertex), static_cast<uint32_t>(verts.size()), mvp, m_particleMat, particleSrv);
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

void EngineApp::requestGameOver()
{
  m_gameOverRequested = true;
}

void EngineApp::requestVictory()
{
  m_victoryRequested = true;
}

bool EngineApp::consumeGameOverRequested()
{
  const bool v = m_gameOverRequested;
  m_gameOverRequested = false;
  return v;
}

bool EngineApp::consumeVictoryRequested()
{
  const bool v = m_victoryRequested;
  m_victoryRequested = false;
  return v;
}

void EngineApp::clearWorld()
{
  delete m_world;
  m_world = nullptr;
  m_player = kInvalidEntity;
  m_scoreEntity = kInvalidEntity;
  m_menuStart = kInvalidEntity;
  m_menuExit = kInvalidEntity;
  m_menuTitleText.clear();
  m_menuStartText.clear();
  m_menuExitText.clear();
  m_menuHintText.clear();
}

void EngineApp::prepareMenuWorld()
{
  clearWorld();
  if (!m_resources) return;

  m_world = new World();

  struct Glyph
  {
    const char* rows[7];
  };

  auto glyphFor = [](char c) -> Glyph
  {
    switch (c)
    {
    case 'A': return { { " ### ", "#   #", "#   #", "#####", "#   #", "#   #", "#   #" } };
    case 'E': return { { "#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#####" } };
    case 'I': return { { "#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "#####" } };
    case 'N': return { { "#   #", "##  #", "# # #", "#  ##", "#   #", "#   #", "#   #" } };
    case 'O': return { { " ### ", "#   #", "#   #", "#   #", "#   #", "#   #", " ### " } };
    case 'P': return { { "#### ", "#   #", "#   #", "#### ", "#    ", "#    ", "#    " } };
    case 'R': return { { "#### ", "#   #", "#   #", "#### ", "# #  ", "#  # ", "#   #" } };
    case 'S': return { { " ####", "#    ", "#    ", " ### ", "    #", "    #", "#### " } };
    case 'T': return { { "#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  " } };
    case 'X': return { { "#   #", "#   #", " # # ", "  #  ", " # # ", "#   #", "#   #" } };
    case ' ': return { { "     ", "     ", "     ", "     ", "     ", "     ", "     " } };
    default:  return { { "?????", "?????", "?????", "?????", "?????", "?????", "?????" } };
    }
  };

  auto spawnBlockText = [&](const char* text, float centerX, float centerY, float pixel, float gap,
                            float r, float g, float b, float a) -> std::vector<EntityId>
  {
    std::vector<EntityId> out;
    if (!m_world) return out;
    if (!text) return out;

    int len = 0;
    for (const char* p = text; *p; ++p) ++len;

    const float charW = 5.0f * pixel + 4.0f * gap;
    const float advance = charW + (3.0f * gap);
    const float totalW = (len > 0) ? (len * advance - (3.0f * gap)) : 0.0f;
    const float startX = centerX - totalW * 0.5f;

    float penX = startX;
    for (const char* p = text; *p; ++p)
    {
      const Glyph gl = glyphFor(*p);

      // Build 5x7 pixels. Row 0 is top.
      for (int row = 0; row < 7; ++row)
      {
        const char* rr = gl.rows[row];
        for (int col = 0; col < 5; ++col)
        {
          const char ch = rr[col];
          if (ch != '#') continue;

          const EntityId e = m_world->createEntity();

          Transform t{};
          t.px = penX + col * (pixel + gap) + pixel * 0.5f;
          t.py = centerY + (3 - row) * (pixel + gap); // center around Y
          t.pz = 0.0f;
          t.sx = pixel;
          t.sy = pixel;
          t.sz = 1.0f;
          m_world->add<Transform>(e, t);

          MeshRenderer mr{};
          mr.mesh = m_quadMesh;
          mr.material = m_meshColorMat;
          mr.tintR = r; mr.tintG = g; mr.tintB = b; mr.tintA = a;
          m_world->add<MeshRenderer>(e, mr);
          out.push_back(e);
        }
      }

      penX += advance;
    }

    return out;
  };

  // Background panel (subtle)
  {
    const EntityId bg = m_world->createEntity();
    Transform t{};
    t.px = 0.0f;
    t.py = 0.0f;
    t.pz = 0.0f;
    t.sx = 1.40f;
    t.sy = 0.80f;
    t.sz = 1.0f;
    m_world->add<Transform>(bg, t);

    MeshRenderer mr{};
    mr.mesh = m_quadMesh;
    mr.material = m_meshColorMat;
    mr.tintR = 0.10f; mr.tintG = 0.14f; mr.tintB = 0.22f; mr.tintA = 0.65f;
    m_world->add<MeshRenderer>(bg, mr);
  }

  // Title
  m_menuTitleText = spawnBlockText("TRON", 0.0f, 0.52f, 0.030f, 0.006f, 0.25f, 0.95f, 1.0f, 0.95f);

  // "Buttons" (we'll highlight selection via tint).
  auto makeButton = [&](float y) -> EntityId
  {
    const EntityId e = m_world->createEntity();
    Transform t{};
    t.px = 0.0f;
    t.py = y;
    t.pz = 0.0f;
    t.sx = 0.55f;
    t.sy = 0.12f;
    t.sz = 1.0f;
    m_world->add<Transform>(e, t);

    MeshRenderer mr{};
    mr.mesh = m_quadMesh;
    mr.material = m_meshColorMat;
    mr.tintR = 0.25f; mr.tintG = 0.25f; mr.tintB = 0.25f; mr.tintA = 0.95f;
    m_world->add<MeshRenderer>(e, mr);
    return e;
  };

  m_menuStart = makeButton(0.12f);
  m_menuExit = makeButton(-0.12f);

  // Labels
  m_menuStartText = spawnBlockText("START", 0.0f, 0.12f, 0.020f, 0.0045f, 0.10f, 0.10f, 0.10f, 0.98f);
  m_menuExitText  = spawnBlockText("EXIT",  0.0f, -0.12f, 0.020f, 0.0045f, 0.10f, 0.10f, 0.10f, 0.98f);
  m_menuHintText  = spawnBlockText("PRESS ENTER", 0.0f, -0.46f, 0.014f, 0.0035f, 0.80f, 0.85f, 0.95f, 0.80f);

  setMenuSelection(0);
}

void EngineApp::setMenuSelection(int selectedIndex)
{
  if (!m_world) return;
  if (m_menuStart == kInvalidEntity || m_menuExit == kInvalidEntity) return;

  auto* startMr = m_world->tryGet<MeshRenderer>(m_menuStart);
  auto* exitMr = m_world->tryGet<MeshRenderer>(m_menuExit);
  if (!startMr || !exitMr) return;

  const auto setTint = [](MeshRenderer& mr, float r, float g, float b, float a)
  {
    mr.tintR = r; mr.tintG = g; mr.tintB = b; mr.tintA = a;
  };

  const bool startSel = (selectedIndex == 0);
  if (startSel)
  {
    setTint(*startMr, 0.20f, 0.95f, 0.45f, 0.98f); // Start highlighted (green)
    setTint(*exitMr,  0.30f, 0.30f, 0.30f, 0.90f); // Exit muted
  }
  else
  {
    setTint(*startMr, 0.30f, 0.30f, 0.30f, 0.90f);
    setTint(*exitMr,  0.95f, 0.25f, 0.25f, 0.98f); // Exit highlighted (red)
  }

  auto tintEntities = [&](const std::vector<EntityId>& ents, float r, float g, float b, float a)
  {
    for (const EntityId e : ents)
    {
      if (auto* mr = m_world->tryGet<MeshRenderer>(e))
      {
        mr->tintR = r; mr->tintG = g; mr->tintB = b; mr->tintA = a;
      }
    }
  };

  if (startSel)
  {
    tintEntities(m_menuStartText, 0.05f, 0.08f, 0.05f, 0.98f);
    tintEntities(m_menuExitText,  0.10f, 0.10f, 0.10f, 0.80f);
  }
  else
  {
    tintEntities(m_menuStartText, 0.10f, 0.10f, 0.10f, 0.80f);
    tintEntities(m_menuExitText,  0.10f, 0.05f, 0.05f, 0.98f);
  }
}

void EngineApp::prepareGameplayWorld()
{
  clearWorld();
  if (!m_resources) return;

  m_world = new World();

  GameWorld builder;
  const GameWorldRefs refs = builder.build(*m_world, *m_resources);
  m_player = refs.player;
  m_scoreEntity = refs.scoreEntity;

  m_gameOverRequested = false;
  m_victoryRequested = false;
}

EntityId EngineApp::spawnTrailSegment(EntityId owner, float x, float y, float size, float ttl,
                                      float r, float g, float b)
{
  if (!m_world) return kInvalidEntity;

  const EntityId seg = m_world->createEntity();

  Transform t{};
  t.px = x;
  t.py = y;
  t.pz = 0.0f;
  t.sx = size;
  t.sy = size;
  t.sz = 1.0f;
  m_world->add<Transform>(seg, t);

  MeshRenderer mr{};
  mr.mesh = m_quadMesh;
  mr.material = m_meshColorMat;
  mr.tintR = r; mr.tintG = g; mr.tintB = b; mr.tintA = 1.0f;
  m_world->add<MeshRenderer>(seg, mr);

  m_world->add<Collider>(seg, Collider::makeAabb(0.5f, 0.5f, 0.01f));
  m_world->add<TrailTag>(seg, TrailTag{});

  TrailSegment ts{};
  ts.owner = owner;
  ts.ttl = ttl;
  ts.ttlMax = ttl;
  ts.spawnTime = m_time ? m_time->totalSeconds() : 0.0f;
  m_world->add<TrailSegment>(seg, ts);

  return seg;
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

  delete m_resources;
  m_resources = nullptr;

  delete m_trailSystem;
  m_trailSystem = nullptr;

  clearWorld();
  m_testEntities.clear();

  delete m_input;
  m_input = nullptr;

  delete m_time;
  m_time = nullptr;
}

