#pragma once

#include <cstdint>

class World;
class D3D11Renderer;

class RenderSystem
{
public:
  // Step 5: render all entities with Transform + MeshRenderer.
  // Returns draw count.
  int render(World& world, D3D11Renderer& renderer, int32_t viewportWidth, int32_t viewportHeight);
};

