#include "D3D11Renderer.hpp"

#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <array>
#include <string>
#include <cstring>

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

static Microsoft::WRL::ComPtr<ID3DBlob> compileFromFile(
  const std::wstring& filePath,
  const char* entryPoint,
  const char* target)
{
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
  Microsoft::WRL::ComPtr<ID3DBlob> errors;

  const HRESULT hr = D3DCompileFromFile(
    filePath.c_str(),
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    entryPoint,
    target,
    flags,
    0,
    bytecode.GetAddressOf(),
    errors.GetAddressOf()
  );

  if (FAILED(hr))
  {
    if (errors)
    {
      const char* err = static_cast<const char*>(errors->GetBufferPointer());
      OutputDebugStringA(err);
    }
    debugOut(L"[D3D11Renderer] Shader compile failed.\n");
    return {};
  }

  return bytecode;
}

bool D3D11Renderer::init(const EngineConfig& cfg)
{
  if (!createDeviceAndSwapChain(cfg)) return false;
  m_width = cfg.width;
  m_height = cfg.height;
  if (!createBackBufferTargets(cfg.width, cfg.height)) return false;
  if (!createTestTriangleResources(cfg)) return false;
  return true;
}

void D3D11Renderer::shutdown()
{
  if (m_context)
  {
    m_context->ClearState();
    m_context->Flush();
  }

  destroyTestTriangleResources();

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

bool D3D11Renderer::createTestTriangleResources(const EngineConfig& cfg)
{
  const std::wstring shaderPath = std::wstring(cfg.assetsRoot) + L"\\Shaders\\Triangle.hlsl";

  Microsoft::WRL::ComPtr<ID3DBlob> vsBytecode = compileFromFile(shaderPath, "VSMain", "vs_5_0");
  if (!vsBytecode) return false;

  Microsoft::WRL::ComPtr<ID3DBlob> psBytecode = compileFromFile(shaderPath, "PSMain", "ps_5_0");
  if (!psBytecode) return false;

  HRESULT hr = m_device->CreateVertexShader(
    vsBytecode->GetBufferPointer(),
    vsBytecode->GetBufferSize(),
    nullptr,
    m_vs.GetAddressOf()
  );
  if (FAILED(hr)) return false;

  hr = m_device->CreatePixelShader(
    psBytecode->GetBufferPointer(),
    psBytecode->GetBufferSize(),
    nullptr,
    m_ps.GetAddressOf()
  );
  if (FAILED(hr)) return false;

  const D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  hr = m_device->CreateInputLayout(
    layout,
    static_cast<UINT>(std::size(layout)),
    vsBytecode->GetBufferPointer(),
    vsBytecode->GetBufferSize(),
    m_inputLayout.GetAddressOf()
  );
  if (FAILED(hr)) return false;

  struct Vertex
  {
    float px, py, pz;
    float r, g, b, a;
  };

  const Vertex verts[3] = {
    {  0.0f,  0.5f, 0.0f, 1.f, 0.f, 0.f, 1.f },
    {  0.5f, -0.5f, 0.0f, 0.f, 1.f, 0.f, 1.f },
    { -0.5f, -0.5f, 0.0f, 0.f, 0.f, 1.f, 1.f },
  };

  D3D11_BUFFER_DESC vbDesc{};
  vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vbDesc.ByteWidth = static_cast<UINT>(sizeof(verts));
  vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

  D3D11_SUBRESOURCE_DATA vbData{};
  vbData.pSysMem = verts;

  hr = m_device->CreateBuffer(&vbDesc, &vbData, m_vb.GetAddressOf());
  if (FAILED(hr)) return false;

  struct alignas(16) CBPerFrame
  {
    DirectX::XMFLOAT4X4 mvp;
  };

  D3D11_BUFFER_DESC cbDesc{};
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.ByteWidth = sizeof(CBPerFrame);
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  hr = m_device->CreateBuffer(&cbDesc, nullptr, m_cbPerFrame.GetAddressOf());
  if (FAILED(hr)) return false;

  return true;
}

void D3D11Renderer::destroyTestTriangleResources()
{
  m_cbPerFrame.Reset();
  m_vb.Reset();
  m_inputLayout.Reset();
  m_ps.Reset();
  m_vs.Reset();
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

void D3D11Renderer::drawTestTriangle(float totalSeconds, float translateX, float translateY)
{
  if (!m_context || !m_vs || !m_ps || !m_inputLayout || !m_vb || !m_cbPerFrame) return;

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

  struct alignas(16) CBPerFrame
  {
    DirectX::XMFLOAT4X4 mvp;
  };

  D3D11_MAPPED_SUBRESOURCE mapped{};
  if (SUCCEEDED(m_context->Map(m_cbPerFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
  {
    const float t = totalSeconds;
    const DirectX::XMMATRIX rot = DirectX::XMMatrixRotationZ(t);
    const DirectX::XMMATRIX tr = DirectX::XMMatrixTranslation(translateX, translateY, 0.0f);
    const DirectX::XMMATRIX mvp = rot * tr;
    CBPerFrame cb{};
    DirectX::XMStoreFloat4x4(&cb.mvp, mvp);
    std::memcpy(mapped.pData, &cb, sizeof(CBPerFrame));
    m_context->Unmap(m_cbPerFrame.Get(), 0);
  }

  const UINT stride = sizeof(float) * (3 + 4);
  const UINT offset = 0;
  ID3D11Buffer* vb = m_vb.Get();

  m_context->IASetInputLayout(m_inputLayout.Get());
  m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

  m_context->VSSetShader(m_vs.Get(), nullptr, 0);
  ID3D11Buffer* cbs[] = { m_cbPerFrame.Get() };
  m_context->VSSetConstantBuffers(0, 1, cbs);

  m_context->PSSetShader(m_ps.Get(), nullptr, 0);

  m_context->Draw(3, 0);
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

