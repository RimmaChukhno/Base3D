#pragma once

#include <cstdint>

using MeshHandle = uint32_t;
using MaterialHandle = uint32_t;

inline constexpr MeshHandle kInvalidMesh = 0xFFFFFFFFu;
inline constexpr MaterialHandle kInvalidMaterial = 0xFFFFFFFFu;

