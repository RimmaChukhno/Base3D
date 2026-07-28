#include "D3D11Renderer.hpp"

#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <array>
#include <string>
#include <cstring>
#include <cstdint>

#include <DirectXMath.h>

static HWND toHwnd(void* windowHandle)
{
  return reinterpret_cast<HWND>(windowHandle);
}

static void debugOut(const wchar_t* msg)
{
#if defined(_DEBUG)
  OutputDebugStringW(msg);
#else
  (void)msg;
#endif
}

bool D3D11Renderer::init(const EngineConfig& cfg)
{
  if (!createDeviceAndSwapChain(cfg)) return false;
  m_width = cfg.width;
  m_height = cfg.height;
  if (!createBackBufferTargets(cfg.width, cfg.height)) return false;
  if (!createDefaultResources(cfg)) return false;
  return true;
}

void D3D11Renderer::shutdown()
{
  if (m_context)
  {
    m_context->ClearState();
    m_context->Flush();
  }

  destroyDefaultResources();

  m_dsv.Reset();
  m_depthTex.Reset();
  m_rtv.Reset();
  m_swapChain.Reset();
  m_context.Reset();
  m_device.Reset();
}

bool D3D11Renderer::createDeviceAndSwapChain(const EngineConfig& cfg)
{
  DXGI_SWAP_CHAIN_DESC scd{};
  scd.BufferCount = 1;
  scd.BufferDesc.Width = static_cast<UINT>(cfg.width);
  scd.BufferDesc.Height = static_cast<UINT>(cfg.height);
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferDesc.RefreshRate.Numerator = 60;
  scd.BufferDesc.RefreshRate.Denominator = 1;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = toHwnd(cfg.windowHandle);
  scd.SampleDesc.Count = 1;
  scd.SampleDesc.Quality = 0;
  scd.Windowed = TRUE;
  scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT flags = 0;
  if (cfg.enableValidation)
  {
    flags |= D3D11_CREATE_DEVICE_DEBUG;
  }

  const std::array<D3D_FEATURE_LEVEL, 2> featureLevels = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0
  };
  D3D_FEATURE_LEVEL createdFeatureLevel{};

  HRESULT hr = D3D11CreateDeviceAndSwapChain(
    nullptr,
    D3D_DRIVER_TYPE_HARDWARE,
    nullptr,
    flags,
    featureLevels.data(),
    static_cast<UINT>(featureLevels.size()),
    D3D11_SDK_VERSION,
    &scd,
    m_swapChain.GetAddressOf(),
    m_device.GetAddressOf(),
    &createdFeatureLevel,
    m_context.GetAddressOf()
  );

  if (FAILED(hr))
  {
    // Fallback: try without debug layer (common when Graphics Tools is not installed).
    if (cfg.enableValidation)
    {
      flags &= ~D3D11_CREATE_DEVICE_DEBUG;
      hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels.data(),
        static_cast<UINT>(featureLevels.size()),
        D3D11_SDK_VERSION,
        &scd,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &createdFeatureLevel,
        m_context.GetAddressOf()
      );
    }
  }

  return SUCCEEDED(hr);
}

bool D3D11Renderer::createBackBufferTargets(int32_t width, int32_t height)
{
  Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
  HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
  if (FAILED(hr)) return false;

  hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());
  if (FAILED(hr)) return false;

  D3D11_TEXTURE2D_DESC depthDesc{};
  depthDesc.Width = static_cast<UINT>(width);
  depthDesc.Height = static_cast<UINT>(height);
  depthDesc.MipLevels = 1;
  depthDesc.ArraySize = 1;
  depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthDesc.SampleDesc.Count = 1;
  depthDesc.SampleDesc.Quality = 0;
  depthDesc.Usage = D3D11_USAGE_DEFAULT;
  depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

  hr = m_device->CreateTexture2D(&depthDesc, nullptr, m_depthTex.GetAddressOf());
  if (FAILED(hr)) return false;

  hr = m_device->CreateDepthStencilView(m_depthTex.Get(), nullptr, m_dsv.GetAddressOf());
  if (FAILED(hr)) return false;

  ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
  m_context->OMSetRenderTargets(1, rtvs, m_dsv.Get());

  D3D11_VIEWPORT vp{};
  vp.TopLeftX = 0.0f;
  vp.TopLeftY = 0.0f;
  vp.Width = static_cast<float>(width);
  vp.Height = static_cast<float>(height);
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  m_context->RSSetViewports(1, &vp);

  return true;
}

MeshHandle D3D11Renderer::createMesh(
  const void* vertices,
  uint32_t vertexStride,
  uint32_t vertexCount,
  const void* indices,
  DXGI_FORMAT indexFormat,
  uint32_t indexCount)
{
  if (!m_device) return kInvalidMesh;
  if (!vertices || vertexStride == 0 || vertexCount == 0) return kInvalidMesh;

  Mesh mesh{};
  mesh.vertexStride = vertexStride;
  mesh.vertexCount = vertexCount;
  mesh.indexFormat = indexFormat;
  mesh.indexCount = indexCount;

  D3D11_BUFFER_DESC vbDesc{};
  vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vbDesc.ByteWidth = vertexStride * vertexCount;
  vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

  D3D11_SUBRESOURCE_DATA vbData{};
  vbData.pSysMem = vertices;

  HRESULT hr = m_device->CreateBuffer(&vbDesc, &vbData, mesh.vb.GetAddressOf());
  if (FAILED(hr)) return kInvalidMesh;

  if (indices && indexCount > 0)
  {
    const uint32_t indexStride = (indexFormat == DXGI_FORMAT_R16_UINT) ? 2u : 4u;

    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.ByteWidth = indexStride * indexCount;
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA ibData{};
    ibData.pSysMem = indices;

    hr = m_device->CreateBuffer(&ibDesc, &ibData, mesh.ib.GetAddressOf());
    if (FAILED(hr)) return kInvalidMesh;
  }

  const MeshHandle handle = static_cast<MeshHandle>(m_meshes.size());
  m_meshes.push_back(std::move(mesh));
  return handle;
}

MaterialHandle D3D11Renderer::createMaterial(const ShaderProgram& program)
{
  Material m{};
  m.program = program;
  const MaterialHandle h = static_cast<MaterialHandle>(m_materials.size());
  m_materials.push_back(std::move(m));
  return h;
}

void D3D11Renderer::beginFrame()
{
  // Reserved for future per-frame setup.
}

void D3D11Renderer::endFrame()
{
  // Reserved for future per-frame teardown.
}

void D3D11Renderer::clear(float r, float g, float b, float a)
{
  const float color[4] = { r, g, b, a };
  if (m_rtv) m_context->ClearRenderTargetView(m_rtv.Get(), color);
  if (m_dsv) m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

const Mesh* D3D11Renderer::tryGetMesh(MeshHandle h) const
{
  if (h == kInvalidMesh) return nullptr;
  if (h >= m_meshes.size()) return nullptr;
  return &m_meshes[h];
}

const Material* D3D11Renderer::tryGetMaterial(MaterialHandle h) const
{
  if (h == kInvalidMaterial) return nullptr;
  if (h >= m_materials.size()) return nullptr;
  return &m_materials[h];
}

void D3D11Renderer::drawMesh(MeshHandle meshH, MaterialHandle matH, const float* mvpRowMajor4x4, const float* tintRGBA)
{
  if (!m_context || !m_cbPerObject) return;
  const Mesh* mesh = tryGetMesh(meshH);
  const Material* mat = tryGetMaterial(matH);
  if (!mesh || !mat) return;
  if (!mat->program.vs || !mat->program.ps || !mat->program.inputLayout) return;
  if (!mesh->vb) return;

  ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
  m_context->OMSetRenderTargets(1, rtvs, m_dsv.Get());

  D3D11_VIEWPORT vp{};
  vp.TopLeftX = 0.0f;
  vp.TopLeftY = 0.0f;
  vp.Width = static_cast<float>(m_width);
  vp.Height = static_cast<float>(m_height);
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  m_context->RSSetViewports(1, &vp);

  struct alignas(16) PerObject
  {
    float mvp[16];
    float tint[4];
  };

  D3D11_MAPPED_SUBRESOURCE mapped{};
  if (SUCCEEDED(m_context->Map(m_cbPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
  {
    PerObject cb{};
    std::memcpy(cb.mvp, mvpRowMajor4x4, sizeof(cb.mvp));
    std::memcpy(cb.tint, tintRGBA, sizeof(cb.tint));
    std::memcpy(mapped.pData, &cb, sizeof(PerObject));
    m_context->Unmap(m_cbPerObject.Get(), 0);
  }

  const UINT stride = mesh->vertexStride;
  const UINT offset = 0;
  ID3D11Buffer* vb = mesh->vb.Get();

  m_context->IASetInputLayout(mat->program.inputLayout.Get());
  m_context->IASetPrimitiveTopology(mesh->topology);
  m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

  if (mesh->ib && mesh->indexCount > 0)
  {
    m_context->IASetIndexBuffer(mesh->ib.Get(), mesh->indexFormat, 0);
  }

  ID3D11Buffer* cbs[] = { m_cbPerObject.Get() };
  m_context->VSSetConstantBuffers(0, 1, cbs);

  m_context->VSSetShader(mat->program.vs.Get(), nullptr, 0);
  m_context->PSSetShader(mat->program.ps.Get(), nullptr, 0);

  if (mesh->ib && mesh->indexCount > 0)
  {
    m_context->DrawIndexed(mesh->indexCount, 0, 0);
  }
  else
  {
    m_context->Draw(mesh->vertexCount, 0);
  }
}

void D3D11Renderer::drawDebugLines(const void* vertices, uint32_t vertexStride, uint32_t vertexCount,
                                  const float* mvpRowMajor4x4)
{
  if (!m_context || !m_device || !m_cbPerObject) return;
  if (!vertices || vertexStride == 0 || vertexCount == 0) return;
  if (m_defaultMaterial == kInvalidMaterial) return;

  const Material* mat = tryGetMaterial(m_defaultMaterial);
  if (!mat || !mat->program.vs || !mat->program.ps || !mat->program.inputLayout) return;

  // Ensure dynamic VB capacity.
  const uint32_t neededBytes = vertexStride * vertexCount;
  if (!m_debugLineVB || m_debugLineVBBytes < neededBytes)
  {
    m_debugLineVB.Reset();
    m_debugLineVBBytes = 0;

    D3D11_BUFFER_DESC desc{};
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = std::max<uint32_t>(neededBytes, 64 * 1024);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(m_device->CreateBuffer(&desc, nullptr, m_debugLineVB.GetAddressOf())))
    {
      return;
    }
    m_debugLineVBBytes = desc.ByteWidth;
  }

  D3D11_MAPPED_SUBRESOURCE mappedVB{};
  if (FAILED(m_context->Map(m_debugLineVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB)))
  {
    return;
  }
  std::memcpy(mappedVB.pData, vertices, neededBytes);
  m_context->Unmap(m_debugLineVB.Get(), 0);

  // Update constant buffer (MVP + tint=1).
  struct alignas(16) PerObject
  {
    float mvp[16];
    float tint[4];
  };
  D3D11_MAPPED_SUBRESOURCE mappedCB{};
  if (SUCCEEDED(m_context->Map(m_cbPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB)))
  {
    PerObject cb{};
    std::memcpy(cb.mvp, mvpRowMajor4x4, sizeof(cb.mvp));
    cb.tint[0] = 1.0f; cb.tint[1] = 1.0f; cb.tint[2] = 1.0f; cb.tint[3] = 1.0f;
    std::memcpy(mappedCB.pData, &cb, sizeof(PerObject));
    m_context->Unmap(m_cbPerObject.Get(), 0);
  }

  ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
  m_context->OMSetRenderTargets(1, rtvs, m_dsv.Get());

  const UINT stride = vertexStride;
  const UINT offset = 0;
  ID3D11Buffer* vb = m_debugLineVB.Get();

  m_context->IASetInputLayout(mat->program.inputLayout.Get());
  m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

  ID3D11Buffer* cbs[] = { m_cbPerObject.Get() };
  m_context->VSSetConstantBuffers(0, 1, cbs);
  m_context->VSSetShader(mat->program.vs.Get(), nullptr, 0);
  m_context->PSSetShader(mat->program.ps.Get(), nullptr, 0);

  m_context->Draw(vertexCount, 0);
}

void D3D11Renderer::present()
{
  if (m_swapChain) m_swapChain->Present(1, 0);
}

void D3D11Renderer::resize(int32_t width, int32_t height)
{
  if (!m_swapChain || !m_device || !m_context) return;
  if (width <= 0 || height <= 0) return;

  m_width = width;
  m_height = height;

  ID3D11RenderTargetView* nullRtvs[] = { nullptr };
  m_context->OMSetRenderTargets(1, nullRtvs, nullptr);

  m_dsv.Reset();
  m_depthTex.Reset();
  m_rtv.Reset();

  const HRESULT hr = m_swapChain->ResizeBuffers(
    0,
    static_cast<UINT>(width),
    static_cast<UINT>(height),
    DXGI_FORMAT_UNKNOWN,
    0
  );

  if (FAILED(hr))
  {
    debugOut(L"[D3D11Renderer] ResizeBuffers failed.\n");
    return;
  }

  (void)createBackBufferTargets(width, height);
}

bool D3D11Renderer::createDefaultResources(const EngineConfig& cfg)
{
  if (!m_device || !m_context) return false;

  ShaderManager shaders(m_device.Get());

  const D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  const std::wstring shaderPath = std::wstring(cfg.assetsRoot) + L"\\Shaders\\MeshColor.hlsl";
  ShaderProgram program = shaders.createProgramFromFile(
    shaderPath,
    "VSMain",
    "PSMain",
    layout,
    static_cast<uint32_t>(std::size(layout))
  );
  if (!program.vs || !program.ps || !program.inputLayout) return false;

  m_defaultMaterial = createMaterial(program);

  struct Vertex
  {
    float px, py, pz;
    float r, g, b, a;
  };

  const Vertex triVerts[3] = {
    {  0.0f,  0.5f, 0.0f, 1.f, 0.f, 0.f, 1.f },
    {  0.5f, -0.5f, 0.0f, 0.f, 1.f, 0.f, 1.f },
    { -0.5f, -0.5f, 0.0f, 0.f, 0.f, 1.f, 1.f },
  };
  m_defaultTriangle = createMesh(triVerts, sizeof(Vertex), 3, nullptr, DXGI_FORMAT_UNKNOWN, 0);

  const Vertex quadVerts[4] = {
    { -0.5f,  0.5f, 0.0f, 1.f, 1.f, 0.f, 1.f },
    {  0.5f,  0.5f, 0.0f, 0.f, 1.f, 1.f, 1.f },
    {  0.5f, -0.5f, 0.0f, 1.f, 0.f, 1.f, 1.f },
    { -0.5f, -0.5f, 0.0f, 1.f, 0.5f, 0.2f, 1.f },
  };
  const uint16_t quadIdx[6] = { 0, 1, 2, 0, 2, 3 };
  m_defaultQuad = createMesh(quadVerts, sizeof(Vertex), 4, quadIdx, DXGI_FORMAT_R16_UINT, 6);

  struct alignas(16) PerObject
  {
    float mvp[16];
    float tint[4];
  };

  D3D11_BUFFER_DESC cbDesc{};
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.ByteWidth = sizeof(PerObject);
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  HRESULT hr = m_device->CreateBuffer(&cbDesc, nullptr, m_cbPerObject.GetAddressOf());
  if (FAILED(hr)) return false;

  return m_defaultTriangle != kInvalidMesh && m_defaultQuad != kInvalidMesh && m_defaultMaterial != kInvalidMaterial;
}

void D3D11Renderer::destroyDefaultResources()
{
  m_cbPerObject.Reset();
  m_debugLineVB.Reset();
  m_debugLineVBBytes = 0;
  m_materials.clear();
  m_meshes.clear();
  m_defaultTriangle = kInvalidMesh;
  m_defaultQuad = kInvalidMesh;
  m_defaultMaterial = kInvalidMaterial;
}

