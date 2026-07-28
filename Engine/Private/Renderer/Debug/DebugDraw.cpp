#include "DebugDraw.hpp"

void DebugDraw::line(float x0, float y0, float z0, float x1, float y1, float z1,
                     float r, float g, float b, float a)
{
  m_vertices.push_back(DebugVertex{ x0, y0, z0, r, g, b, a });
  m_vertices.push_back(DebugVertex{ x1, y1, z1, r, g, b, a });
}

void DebugDraw::aabb(float minX, float minY, float minZ, float maxX, float maxY, float maxZ,
                     float r, float g, float b, float a)
{
  // 12 edges => 24 vertices.
  // Bottom rectangle (z=minZ)
  line(minX, minY, minZ, maxX, minY, minZ, r, g, b, a);
  line(maxX, minY, minZ, maxX, maxY, minZ, r, g, b, a);
  line(maxX, maxY, minZ, minX, maxY, minZ, r, g, b, a);
  line(minX, maxY, minZ, minX, minY, minZ, r, g, b, a);

  // Top rectangle (z=maxZ)
  line(minX, minY, maxZ, maxX, minY, maxZ, r, g, b, a);
  line(maxX, minY, maxZ, maxX, maxY, maxZ, r, g, b, a);
  line(maxX, maxY, maxZ, minX, maxY, maxZ, r, g, b, a);
  line(minX, maxY, maxZ, minX, minY, maxZ, r, g, b, a);

  // Vertical edges
  line(minX, minY, minZ, minX, minY, maxZ, r, g, b, a);
  line(maxX, minY, minZ, maxX, minY, maxZ, r, g, b, a);
  line(maxX, maxY, minZ, maxX, maxY, maxZ, r, g, b, a);
  line(minX, maxY, minZ, minX, maxY, maxZ, r, g, b, a);
}

