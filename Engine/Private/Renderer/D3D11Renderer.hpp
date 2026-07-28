#pragma once

#include "EngineAPI.hpp"

#include <wrl/client.h>

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "Renderer/Resources/Handles.hpp"
#include "Renderer/Resources/Mesh.hpp"
#include "Renderer/Resources/Material.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"

#include <vector>

class D3D11Renderer
{
public:
  bool init(const EngineConfig& cfg);
  void shutdown();

  void beginFrame();
  void endFrame();
  void clear(float r, float g, float b, float a);
  void present();
  void resize(int32_t width, int32_t height);

  // Resource creation for Step 5 (temporary until ResourceManager).
  MeshHandle createMesh(const void* vertices, uint32_t vertexStride, uint32_t vertexCount,
                        const void* indices, DXGI_FORMAT indexFormat, uint32_t indexCount);
  MaterialHandle createMaterial(const ShaderProgram& program);

  const Mesh* tryGetMesh(MeshHandle h) const;
  const Material* tryGetMaterial(MaterialHandle h) const;

  // Default built-ins used by the student engine sample.
  MeshHandle defaultTriangleMesh() const { return m_defaultTriangle; }
  MeshHandle defaultQuadMesh() const { return m_defaultQuad; }
  MaterialHandle defaultColorMaterial() const { return m_defaultMaterial; }

  // Draw call used by RenderSystem.
  void drawMesh(MeshHandle mesh, MaterialHandle material, const float* mvpRowMajor4x4, const float* tintRGBA);

private:
  bool createDeviceAndSwapChain(const EngineConfig& cfg);
  bool createBackBufferTargets(int32_t width, int32_t height);
  bool createDefaultResources(const EngineConfig& cfg);
  void destroyDefaultResources();

private:
  Microsoft::WRL::ComPtr<ID3D11Device> m_device;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
  Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTex;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

  std::vector<Mesh> m_meshes;
  std::vector<Material> m_materials;

  MeshHandle m_defaultTriangle = kInvalidMesh;
  MeshHandle m_defaultQuad = kInvalidMesh;
  MaterialHandle m_defaultMaterial = kInvalidMaterial;

  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbPerObject;

  int32_t m_width = 0;
  int32_t m_height = 0;
};

