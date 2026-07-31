#pragma once

#include "Renderer/Resources/Handles.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

using TextureHandle = uint32_t;
inline constexpr TextureHandle kInvalidTexture = 0xFFFFFFFFu;

class D3D11Renderer;

// ResourceManager (Step 10):
// - centralizes creation & caching (dedup) of GPU resources
// - provides stable handles for game systems (Mesh/Material/Texture)
//
// For now it is Engine-private: we don't expose it across the DLL boundary.
class ResourceManager
{
public:
  ResourceManager(ID3D11Device* device, D3D11Renderer& renderer, std::wstring assetsRoot);

  // Built-in primitives (used by sample content)
  MeshHandle getOrCreateTriangleMesh();
  MeshHandle getOrCreateQuadMesh();

  // Built-in materials (from Assets/Shaders)
  MaterialHandle getOrCreateMeshColorMaterial();
  MaterialHandle getOrCreateParticleMaterial();

  // Textures
  TextureHandle getOrCreateWhiteTexture1x1();
  ID3D11ShaderResourceView* srv(TextureHandle h) const;

  // Stats (dedup proof)
  uint32_t meshCount() const { return static_cast<uint32_t>(m_meshCache.size()); }
  uint32_t materialCount() const { return static_cast<uint32_t>(m_materialCache.size()); }
  uint32_t shaderProgramCount() const { return static_cast<uint32_t>(m_shaderCache.size()); }
  uint32_t textureCount() const { return static_cast<uint32_t>(m_textures.size()); }

private:
  struct CacheValMesh { MeshHandle handle = kInvalidMesh; };
  struct CacheValMat  { MaterialHandle handle = kInvalidMaterial; };

  const ShaderProgram& getOrCreateShader(
    const std::wstring& relPath,
    const char* vsEntry,
    const char* psEntry,
    const D3D11_INPUT_ELEMENT_DESC* elems,
    uint32_t elemCount);

  MaterialHandle getOrCreateMaterial(
    const std::wstring& key,
    const ShaderProgram& program);

private:
  ID3D11Device* m_device = nullptr;
  D3D11Renderer* m_renderer = nullptr;
  std::wstring m_assetsRoot;
  ShaderManager m_shaders;

  std::unordered_map<std::wstring, ShaderProgram> m_shaderCache;     // key -> program
  std::unordered_map<std::wstring, CacheValMesh> m_meshCache;        // key -> mesh handle
  std::unordered_map<std::wstring, CacheValMat> m_materialCache;     // key -> material handle

  std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
};

