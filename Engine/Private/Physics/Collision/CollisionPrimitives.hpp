#pragma once

#include <cstdint>

namespace Collision
{
  struct Vec3f
  {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
  };

  inline Vec3f operator+(const Vec3f& a, const Vec3f& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
  inline Vec3f operator-(const Vec3f& a, const Vec3f& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
  inline Vec3f operator*(const Vec3f& a, float s) { return { a.x * s, a.y * s, a.z * s }; }

  inline float dot(const Vec3f& a, const Vec3f& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
  inline float lengthSq(const Vec3f& v) { return dot(v, v); }

  struct Aabb
  {
    Vec3f min;
    Vec3f max;
  };

  struct Sphere
  {
    Vec3f center;
    float radius = 0.5f;
  };

  struct Ray
  {
    Vec3f origin;
    Vec3f dir; // should be normalized for meaningful t
  };
}

