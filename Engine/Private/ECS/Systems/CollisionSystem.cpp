#include "CollisionSystem.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Collider.hpp"
#include "ECS/Components/MeshRenderer.hpp"

#include "Physics/Collision/CollisionPrimitives.hpp"
#include "Physics/Collision/Intersection.hpp"

#include "Renderer/D3D11Renderer.hpp"
#include "Renderer/Debug/DebugDraw.hpp"

#include <DirectXMath.h>
#include <vector>

static void buildAspectMvpRowMajor(float outMvp[16], float aspect)
{
  using namespace DirectX;
  const XMMATRIX aspectFix = XMMatrixScaling(1.0f / (aspect > 0.0001f ? aspect : 1.0f), 1.0f, 1.0f);
  XMFLOAT4X4 m{};
  XMStoreFloat4x4(&m, aspectFix);
  const float* p = reinterpret_cast<const float*>(&m);
  for (int i = 0; i < 16; ++i) outMvp[i] = p[i];
}

static Collision::Aabb makeWorldAabb(const Transform& tr, const Collider& c)
{
  // Step 6 simplification:
  // - Rotation ignored (still useful for many student projects and for broadphase).
  const float hx = c.hx * tr.sx;
  const float hy = c.hy * tr.sy;
  const float hz = c.hz * tr.sz;

  Collision::Aabb a{};
  a.min = { tr.px - hx, tr.py - hy, tr.pz - hz };
  a.max = { tr.px + hx, tr.py + hy, tr.pz + hz };
  return a;
}

CollisionStats CollisionSystem::update(World& world, D3D11Renderer& renderer, int32_t viewportWidth, int32_t viewportHeight,
                                       std::vector<std::pair<EntityId, EntityId>>* outOverlaps,
                                       MaterialHandle debugMaterial)
{
  CollisionStats stats{};
  if (outOverlaps) outOverlaps->clear();

  // Gather colliders (Transform + Collider).
  struct Item
  {
    EntityId e{};
    Collision::Aabb aabb{};
    bool hit = false;
  };

  std::vector<Item> items;
  items.reserve(512);

  // We don't have a 3-component each() yet; use two-component join and probe the third.
  world.each<Transform, Collider>([&](EntityId e, Transform& tr, Collider& col)
  {
    if (col.type != ColliderType::Aabb) return;
    items.push_back(Item{ e, makeWorldAabb(tr, col), false });
  });

  stats.colliderCount = static_cast<int32_t>(items.size());

  // Naive narrowphase O(n^2) for Step 6 (OK for small sets).
  const int n = static_cast<int>(items.size());
  for (int i = 0; i < n; ++i)
  {
    for (int j = i + 1; j < n; ++j)
    {
      stats.pairChecks += 1;
      if (Collision::intersects(items[i].aabb, items[j].aabb))
      {
        items[i].hit = true;
        items[j].hit = true;
        stats.overlaps += 1;
        if (outOverlaps)
        {
          outOverlaps->push_back({ items[i].e, items[j].e });
        }
      }
    }
  }

  // Debug draw + optional color feedback on MeshRenderer.
  DebugDraw dd;
  dd.reserve(static_cast<uint32_t>(items.size()) * 24u);

  for (auto& it : items)
  {
    const bool hit = it.hit;
    const float r = hit ? 1.0f : 0.1f;
    const float g = hit ? 0.1f : 1.0f;
    const float b = 0.1f;
    dd.aabb(it.aabb.min.x, it.aabb.min.y, it.aabb.min.z, it.aabb.max.x, it.aabb.max.y, it.aabb.max.z, r, g, b, 1.0f);

    // If entity has a MeshRenderer, tint it so collisions are obvious without debug lines.
    if (auto* mr = world.tryGet<MeshRenderer>(it.e))
    {
      if (hit)
      {
        mr->tintR = 1.0f; mr->tintG = 0.2f; mr->tintB = 0.2f;
      }
      else
      {
        // Restore a neutral-ish tint (keep alpha).
        mr->tintR = 0.7f; mr->tintG = 0.8f; mr->tintB = 0.9f;
      }
    }
  }

  stats.debugLineVertexCount = static_cast<int32_t>(dd.vertexCount());

  // Flush debug lines through the renderer.
  const float aspect = (viewportHeight > 0) ? (static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight)) : 1.0f;
  float mvp[16]{};
  buildAspectMvpRowMajor(mvp, aspect);
  renderer.drawDebugLines(dd.data(), sizeof(DebugVertex), dd.vertexCount(), mvp, debugMaterial);

  return stats;
}

