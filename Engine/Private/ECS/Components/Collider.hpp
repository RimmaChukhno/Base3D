#pragma once

// Minimal Collider component for Step 6.
// We keep it data-only and independent of the renderer.

enum class ColliderType : int
{
  Aabb = 0,
  Sphere = 1,
};

struct Collider
{
  ColliderType type = ColliderType::Aabb;

  // Local-space parameters (interpreted with Transform).
  // - AABB: half-extents in local space (axis-aligned, rotation ignored for now)
  // - Sphere: radius in local space
  float hx = 0.25f;
  float hy = 0.25f;
  float hz = 0.25f;
  float radius = 0.5f;

  static Collider makeAabb(float halfX, float halfY, float halfZ)
  {
    Collider c{};
    c.type = ColliderType::Aabb;
    c.hx = halfX; c.hy = halfY; c.hz = halfZ;
    return c;
  }

  static Collider makeSphere(float r)
  {
    Collider c{};
    c.type = ColliderType::Sphere;
    c.radius = r;
    return c;
  }
};

