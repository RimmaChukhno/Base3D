#pragma once

#include "Renderer/Resources/Handles.hpp"

struct MeshRenderer
{
  MeshHandle mesh = kInvalidMesh;
  MaterialHandle material = kInvalidMaterial;

  float tintR = 1.0f;
  float tintG = 1.0f;
  float tintB = 1.0f;
  float tintA = 1.0f;
};

