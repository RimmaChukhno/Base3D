#include "RenderSystem.hpp"

#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/MeshRenderer.hpp"

#include "Renderer/D3D11Renderer.hpp"

#include <DirectXMath.h>

static void buildMvpRowMajor(const Transform& tr, float outMvp[16], float aspect)
{
  using namespace DirectX;

  const XMMATRIX S = XMMatrixScaling(tr.sx, tr.sy, tr.sz);
  const XMMATRIX R =
    XMMatrixRotationX(tr.rx) *
    XMMatrixRotationY(tr.ry) *
    XMMatrixRotationZ(tr.rz);
  const XMMATRIX T = XMMatrixTranslation(tr.px, tr.py, tr.pz);

  // For Step 5 we use a very simple camera:
  // - "view" = identity
  // - "proj" = aspect correction in X so shapes remain consistent on resize
  const XMMATRIX aspectFix = XMMatrixScaling(1.0f / (aspect > 0.0001f ? aspect : 1.0f), 1.0f, 1.0f);

  const XMMATRIX M = S * R * T;
  const XMMATRIX MVP = M * aspectFix;

  XMFLOAT4X4 mvp{};
  XMStoreFloat4x4(&mvp, MVP);

  // Row-major copy (matches our HLSL 'row_major float4x4').
  const float* p = reinterpret_cast<const float*>(&mvp);
  for (int i = 0; i < 16; ++i) outMvp[i] = p[i];
}

int RenderSystem::render(World& world, D3D11Renderer& renderer, int32_t viewportWidth, int32_t viewportHeight)
{
  int draws = 0;
  const float aspect = (viewportHeight > 0) ? (static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight)) : 1.0f;

  world.each<Transform, MeshRenderer>([&](EntityId, Transform& tr, MeshRenderer& mr)
  {
    if (mr.mesh == kInvalidMesh || mr.material == kInvalidMaterial) return;

    float mvp[16]{};
    buildMvpRowMajor(tr, mvp, aspect);
    const float tint[4] = { mr.tintR, mr.tintG, mr.tintB, mr.tintA };
    renderer.drawMesh(mr.mesh, mr.material, mvp, tint);
    draws += 1;
  });

  return draws;
}

