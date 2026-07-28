#pragma once

#include <wrl/client.h>

#include <d3d11.h>

#include <cstdint>
#include <string>
#include <vector>

struct ShaderProgram
{
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

class ShaderManager
{
public:
  explicit ShaderManager(ID3D11Device* device);

  ShaderProgram createProgramFromFile(
    const std::wstring& filePath,
    const char* vsEntry,
    const char* psEntry,
    const D3D11_INPUT_ELEMENT_DESC* inputElements,
    uint32_t inputElementCount);

private:
  ID3D11Device* m_device = nullptr;
};

