#pragma once

// Minimal Transform for Step 4.
// We'll replace these with proper math types (Vec3/Quat/Mat4) later.
struct Transform
{
  float px = 0.0f;
  float py = 0.0f;
  float pz = 0.0f;

  float rx = 0.0f;
  float ry = 0.0f;
  float rz = 0.0f; // radians

  float sx = 1.0f;
  float sy = 1.0f;
  float sz = 1.0f;
};

