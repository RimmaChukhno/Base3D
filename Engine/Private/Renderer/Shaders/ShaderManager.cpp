#include "ShaderManager.hpp"

#include <d3dcompiler.h>

#include <Windows.h>

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
    debugOut(L"[ShaderManager] Shader compile failed.\n");
    return {};
  }

  return bytecode;
}

ShaderManager::ShaderManager(ID3D11Device* device)
  : m_device(device)
{
}

ShaderProgram ShaderManager::createProgramFromFile(
  const std::wstring& filePath,
  const char* vsEntry,
  const char* psEntry,
  const D3D11_INPUT_ELEMENT_DESC* inputElements,
  uint32_t inputElementCount)
{
  ShaderProgram program{};
  if (!m_device) return program;

  auto vsBytecode = compileFromFile(filePath, vsEntry, "vs_5_0");
  if (!vsBytecode) return program;

  auto psBytecode = compileFromFile(filePath, psEntry, "ps_5_0");
  if (!psBytecode) return program;

  HRESULT hr = m_device->CreateVertexShader(
    vsBytecode->GetBufferPointer(),
    vsBytecode->GetBufferSize(),
    nullptr,
    program.vs.GetAddressOf()
  );
  if (FAILED(hr)) return {};

  hr = m_device->CreatePixelShader(
    psBytecode->GetBufferPointer(),
    psBytecode->GetBufferSize(),
    nullptr,
    program.ps.GetAddressOf()
  );
  if (FAILED(hr)) return {};

  hr = m_device->CreateInputLayout(
    inputElements,
    inputElementCount,
    vsBytecode->GetBufferPointer(),
    vsBytecode->GetBufferSize(),
    program.inputLayout.GetAddressOf()
  );
  if (FAILED(hr)) return {};

  return program;
}

