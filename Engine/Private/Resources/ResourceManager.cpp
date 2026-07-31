#include "ResourceManager.hpp"

#include "Renderer/D3D11Renderer.hpp"

#include <array>

static std::wstring makeShaderKey(const std::wstring& absPath, const char* vs, const char* ps, uint32_t layoutHash)
{
  std::wstring key = absPath;
  key += L"|";
  // cheap wide conversion (ASCII entry points)
  while (*vs) { key.push_back(static_cast<wchar_t>(*vs++)); }
  key += L"|";
  while (*ps) { key.push_back(static_cast<wchar_t>(*ps++)); }
  key += L"|";
  key += std::to_wstring(layoutHash);
  return key;
}

static uint32_t hashLayout(const D3D11_INPUT_ELEMENT_DESC* elems, uint32_t count)
{
  // Simple FNV-1a over relevant fields (good enough for caching keys).
  uint32_t h = 2166136261u;
  auto fnv = [&](uint32_t v) { h ^= v; h *= 16777619u; };

  for (uint32_t i = 0; i < count; ++i)
  {
    const auto& e = elems[i];
    // semantic name (ascii)
    const char* s = e.SemanticName ? e.SemanticName : "";
    while (*s) { fnv(static_cast<uint8_t>(*s++)); }
    fnv(e.SemanticIndex);
    fnv(static_cast<uint32_t>(e.Format));
    fnv(e.InputSlot);
    fnv(e.AlignedByteOffset);
    fnv(static_cast<uint32_t>(e.InputSlotClass));
    fnv(e.InstanceDataStepRate);
  }
  return h;
}

ResourceManager::ResourceManager(ID3D11Device* device, D3D11Renderer& renderer, std::wstring assetsRoot)
  : m_device(device)
  , m_renderer(&renderer)
  , m_assetsRoot(std::move(assetsRoot))
  , m_shaders(device)
{
  m_textures.reserve(16);
}

const ShaderProgram& ResourceManager::getOrCreateShader(
  const std::wstring& relPath,
  const char* vsEntry,
  const char* psEntry,
  const D3D11_INPUT_ELEMENT_DESC* elems,
  uint32_t elemCount)
{
  const std::wstring absPath = m_assetsRoot + L"\\" + relPath;
  const uint32_t layoutHash = hashLayout(elems, elemCount);
  const std::wstring key = makeShaderKey(absPath, vsEntry, psEntry, layoutHash);

  auto it = m_shaderCache.find(key);
  if (it != m_shaderCache.end())
  {
    return it->second;
  }

  ShaderProgram prog = m_shaders.createProgramFromFile(absPath, vsEntry, psEntry, elems, elemCount);
  auto [insIt, _] = m_shaderCache.emplace(key, std::move(prog));
  return insIt->second;
}

MaterialHandle ResourceManager::getOrCreateMaterial(const std::wstring& key, const ShaderProgram& program)
{
  auto it = m_materialCache.find(key);
  if (it != m_materialCache.end())
  {
    return it->second.handle;
  }

  MaterialHandle h = m_renderer->createMaterial(program);
  m_materialCache.emplace(key, CacheValMat{ h });
  return h;
}

MeshHandle ResourceManager::getOrCreateTriangleMesh()
{
  const std::wstring key = L"builtin:triangle";
  auto it = m_meshCache.find(key);
  if (it != m_meshCache.end()) return it->second.handle;

  struct Vertex { float px, py, pz; float r, g, b, a; };
  const Vertex triVerts[3] = {
    {  0.0f,  0.5f, 0.0f, 1.f, 0.f, 0.f, 1.f },
    {  0.5f, -0.5f, 0.0f, 0.f, 1.f, 0.f, 1.f },
    { -0.5f, -0.5f, 0.0f, 0.f, 0.f, 1.f, 1.f },
  };

  const MeshHandle h = m_renderer->createMesh(triVerts, sizeof(Vertex), 3, nullptr, DXGI_FORMAT_UNKNOWN, 0);
  m_meshCache.emplace(key, CacheValMesh{ h });
  return h;
}

MeshHandle ResourceManager::getOrCreateQuadMesh()
{
  const std::wstring key = L"builtin:quad";
  auto it = m_meshCache.find(key);
  if (it != m_meshCache.end()) return it->second.handle;

  struct Vertex { float px, py, pz; float r, g, b, a; };
  const Vertex quadVerts[4] = {
    { -0.5f,  0.5f, 0.0f, 1.f, 1.f, 0.f, 1.f },
    {  0.5f,  0.5f, 0.0f, 0.f, 1.f, 1.f, 1.f },
    {  0.5f, -0.5f, 0.0f, 1.f, 0.f, 1.f, 1.f },
    { -0.5f, -0.5f, 0.0f, 1.f, 0.5f, 0.2f, 1.f },
  };
  const uint16_t quadIdx[6] = { 0, 1, 2, 0, 2, 3 };

  const MeshHandle h = m_renderer->createMesh(quadVerts, sizeof(Vertex), 4, quadIdx, DXGI_FORMAT_R16_UINT, 6);
  m_meshCache.emplace(key, CacheValMesh{ h });
  return h;
}

MaterialHandle ResourceManager::getOrCreateMeshColorMaterial()
{
  const D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  const ShaderProgram& prog = getOrCreateShader(L"Shaders\\MeshColor.hlsl", "VSMain", "PSMain", layout, 2);
  return getOrCreateMaterial(L"mat:MeshColor", prog);
}

MaterialHandle ResourceManager::getOrCreateParticleMaterial()
{
  const D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  const ShaderProgram& prog = getOrCreateShader(L"Shaders\\Particles.hlsl", "VSMain", "PSMain", layout, 3);
  return getOrCreateMaterial(L"mat:Particles", prog);
}

TextureHandle ResourceManager::getOrCreateWhiteTexture1x1()
{
  const std::wstring key = L"tex:white1x1";
  // For textures we just use a materialCache-like trick by storing a nullptr SRV slot in a map,
  // but to keep this minimal, we simply create once and store at index 0.
  if (!m_textures.empty()) return 0;

  const uint32_t white = 0xFFFFFFFFu;

  D3D11_TEXTURE2D_DESC tdesc{};
  tdesc.Width = 1;
  tdesc.Height = 1;
  tdesc.MipLevels = 1;
  tdesc.ArraySize = 1;
  tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  tdesc.SampleDesc.Count = 1;
  tdesc.Usage = D3D11_USAGE_IMMUTABLE;
  tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA tdata{};
  tdata.pSysMem = &white;
  tdata.SysMemPitch = 4;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
  HRESULT hr = m_device->CreateTexture2D(&tdesc, &tdata, tex.GetAddressOf());
  if (FAILED(hr)) return kInvalidTexture;

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
  hr = m_device->CreateShaderResourceView(tex.Get(), nullptr, srv.GetAddressOf());
  if (FAILED(hr)) return kInvalidTexture;

  m_textures.push_back(std::move(srv));
  (void)key;
  return 0;
}

ID3D11ShaderResourceView* ResourceManager::srv(TextureHandle h) const
{
  if (h == kInvalidTexture) return nullptr;
  if (h >= m_textures.size()) return nullptr;
  return m_textures[h].Get();
}

