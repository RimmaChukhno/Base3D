#pragma once

#include <wrl/client.h>

#include <d3d11.h>

#include <cstdint>

struct Mesh
{
  Microsoft::WRL::ComPtr<ID3D11Buffer> vb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> ib; // optional

  uint32_t vertexStride = 0;
  uint32_t vertexCount = 0;

  DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
  uint32_t indexCount = 0;

  D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

