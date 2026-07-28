#pragma once

#include <cstdint>
#include <vector>

// CPU-side debug draw collector (Step 6).
// It stores line vertices and flushes them through D3D11Renderer.
struct DebugVertex
{
  float px, py, pz;
  float r, g, b, a;
};

class D3D11Renderer;

class DebugDraw
{
public:
  void clear() { m_vertices.clear(); }
  void reserve(uint32_t vtx) { m_vertices.reserve(vtx); }

  void line(float x0, float y0, float z0, float x1, float y1, float z1,
            float r, float g, float b, float a);

  void aabb(float minX, float minY, float minZ, float maxX, float maxY, float maxZ,
            float r, float g, float b, float a);

  uint32_t vertexCount() const { return static_cast<uint32_t>(m_vertices.size()); }
  const DebugVertex* data() const { return m_vertices.data(); }

private:
  std::vector<DebugVertex> m_vertices;
};

