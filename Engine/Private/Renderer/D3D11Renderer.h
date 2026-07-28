#pragma once

#include "EngineAPI.h"

#include <wrl/client.h>

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

class D3D11Renderer
{
public:
  bool init(const EngineConfig& cfg);
  void shutdown();

  void beginFrame();
  void endFrame();
  void clear(float r, float g, float b, float a);
  void drawTestTriangle(float totalSeconds, float translateX, float translateY);
  void present();
  void resize(int32_t width, int32_t height);

private:
  bool createDeviceAndSwapChain(const EngineConfig& cfg);
  bool createBackBufferTargets(int32_t width, int32_t height);
  bool createTestTriangleResources(const EngineConfig& cfg);
  void destroyTestTriangleResources();

private:
  Microsoft::WRL::ComPtr<ID3D11Device> m_device;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
  Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTex;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbPerFrame;

  int32_t m_width = 0;
  int32_t m_height = 0;
};

