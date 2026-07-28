#pragma once

#include "Physics/Collision/CollisionPrimitives.hpp"

#include <algorithm>
#include <cmath>

namespace Collision
{
  inline bool intersects(const Aabb& a, const Aabb& b)
  {
    // Separating Axis Theorem for AABBs: overlap on all axes.
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
    return true;
  }

  inline bool intersects(const Sphere& a, const Sphere& b)
  {
    const Vec3f d = a.center - b.center;
    const float r = a.radius + b.radius;
    return lengthSq(d) <= r * r;
  }

  inline bool intersects(const Sphere& s, const Aabb& b)
  {
    // Clamp sphere center to AABB, then compare distance to radius.
    const float cx = std::clamp(s.center.x, b.min.x, b.max.x);
    const float cy = std::clamp(s.center.y, b.min.y, b.max.y);
    const float cz = std::clamp(s.center.z, b.min.z, b.max.z);
    const Vec3f closest{ cx, cy, cz };
    const Vec3f d = s.center - closest;
    return lengthSq(d) <= s.radius * s.radius;
  }

  struct RayHit
  {
    bool hit = false;
    float t = 0.0f;      // distance along ray
    Vec3f point{};       // origin + dir * t
  };

  inline RayHit intersectRayAabb(const Ray& ray, const Aabb& aabb, float tMin = 0.0f, float tMax = 1e30f)
  {
    // Slab method.
    auto inv = [](float v) -> float { return (std::fabs(v) > 1e-8f) ? (1.0f / v) : 1e30f; };
    const float invX = inv(ray.dir.x);
    const float invY = inv(ray.dir.y);
    const float invZ = inv(ray.dir.z);

    float t0 = (aabb.min.x - ray.origin.x) * invX;
    float t1 = (aabb.max.x - ray.origin.x) * invX;
    if (t0 > t1) std::swap(t0, t1);
    tMin = std::max(tMin, t0);
    tMax = std::min(tMax, t1);
    if (tMax < tMin) return {};

    t0 = (aabb.min.y - ray.origin.y) * invY;
    t1 = (aabb.max.y - ray.origin.y) * invY;
    if (t0 > t1) std::swap(t0, t1);
    tMin = std::max(tMin, t0);
    tMax = std::min(tMax, t1);
    if (tMax < tMin) return {};

    t0 = (aabb.min.z - ray.origin.z) * invZ;
    t1 = (aabb.max.z - ray.origin.z) * invZ;
    if (t0 > t1) std::swap(t0, t1);
    tMin = std::max(tMin, t0);
    tMax = std::min(tMax, t1);
    if (tMax < tMin) return {};

    RayHit hit{};
    hit.hit = true;
    hit.t = tMin;
    hit.point = ray.origin + ray.dir * tMin;
    return hit;
  }
}

