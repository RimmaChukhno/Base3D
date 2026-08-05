#include "GameWorld.hpp"

#include "Resources/ResourceManager.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/MeshRenderer.hpp"
#include "ECS/Components/Collider.hpp"
#include "ECS/Components/ScriptComponent.hpp"

#include "Game/Components/PlayerTag.hpp"
#include "Game/Components/EnemyTag.hpp"
#include "Game/Components/WallTag.hpp"
#include "Game/Components/ScoreComponent.hpp"

#include "Game/Scripts/PlayerControllerScript.hpp"
#include "Game/Scripts/EnemyAIScript.hpp"
#include "Game/Scripts/GameDirectorScript.hpp"

#include <memory>

static void addWall(World& world, ResourceManager& res, float x, float y, float sx, float sy)
{
  const EntityId w = world.createEntity();

  Transform t{};
  t.px = x;
  t.py = y;
  t.pz = 0.0f;
  t.sx = sx;
  t.sy = sy;
  t.sz = 1.0f;
  world.add<Transform>(w, t);

  MeshRenderer mr{};
  mr.mesh = res.getOrCreateQuadMesh();
  mr.material = res.getOrCreateMeshColorMaterial();
  mr.tintR = 0.15f; mr.tintG = 0.85f; mr.tintB = 0.95f; mr.tintA = 1.0f;
  world.add<MeshRenderer>(w, mr);

  // Collider half extents in local space; scale is applied in CollisionSystem via Transform scale.
  world.add<Collider>(w, Collider::makeAabb(0.5f, 0.5f, 0.01f));

  world.add<WallTag>(w, WallTag{});
}

GameWorldRefs GameWorld::build(World& world, ResourceManager& resources)
{
  GameWorldRefs refs{};

  // Arena bounds (approx -1..1). Walls are thick quads.
  addWall(world, resources, 0.0f,  0.98f, 2.2f, 0.08f); // top
  addWall(world, resources, 0.0f, -0.98f, 2.2f, 0.08f); // bottom
  addWall(world, resources, 0.98f, 0.0f,  0.08f, 2.2f); // right
  addWall(world, resources, -0.98f,0.0f,  0.08f, 2.2f); // left

  // Score entity (singleton-ish)
  refs.scoreEntity = world.createEntity();
  world.add<ScoreComponent>(refs.scoreEntity, ScoreComponent{});

  // Game director
  {
    const EntityId director = world.createEntity();
    auto gd = std::make_unique<GameDirectorScript>();
    gd->setScoreEntity(refs.scoreEntity);
    gd->setVictoryTime(30.0f);
    world.add<ScriptComponent>(director, ScriptComponent{ std::move(gd) });
  }

  // Player
  refs.player = world.createEntity();
  {
    Transform t{};
    t.px = -0.4f;
    t.py = 0.0f;
    t.sx = 0.06f;
    t.sy = 0.06f;
    world.add<Transform>(refs.player, t);

    MeshRenderer mr{};
    mr.mesh = resources.getOrCreateQuadMesh();
    mr.material = resources.getOrCreateMeshColorMaterial();
    mr.tintR = 0.3f; mr.tintG = 1.0f; mr.tintB = 0.4f; mr.tintA = 1.0f;
    world.add<MeshRenderer>(refs.player, mr);

    world.add<Collider>(refs.player, Collider::makeAabb(0.5f, 0.5f, 0.01f));
    world.add<PlayerTag>(refs.player, PlayerTag{});
    world.add<ScriptComponent>(refs.player, ScriptComponent{ std::make_unique<PlayerControllerScript>() });
  }

  // One enemy (Step 11.1)
  refs.enemy = world.createEntity();
  {
    Transform t{};
    t.px = 0.4f;
    t.py = 0.0f;
    t.sx = 0.06f;
    t.sy = 0.06f;
    world.add<Transform>(refs.enemy, t);

    MeshRenderer mr{};
    mr.mesh = resources.getOrCreateQuadMesh();
    mr.material = resources.getOrCreateMeshColorMaterial();
    mr.tintR = 1.0f; mr.tintG = 0.35f; mr.tintB = 0.35f; mr.tintA = 1.0f;
    world.add<MeshRenderer>(refs.enemy, mr);

    world.add<Collider>(refs.enemy, Collider::makeAabb(0.5f, 0.5f, 0.01f));
    world.add<EnemyTag>(refs.enemy, EnemyTag{});

    auto ai = std::make_unique<EnemyAIScript>();
    ai->setTarget(refs.player);
    world.add<ScriptComponent>(refs.enemy, ScriptComponent{ std::move(ai) });
  }

  return refs;
}

