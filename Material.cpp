#include "Material.h"

/// <summary>
/// Constructor for creating a material
/// </summary>
/// <param name="_colorTint">The color tint of the material</param>
/// <param name="_vertexShader">The vertex shader for the material</param>
/// <param name="_pixelShader">the pixel shader for the material</param>
Material::Material(DirectX::XMFLOAT4 _colorTint, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness, float _uvScale, DirectX::XMFLOAT2 _uvOffset)
{
	colorTint = _colorTint;
	vertexShader = _vertexShader;
	pixelShader = _pixelShader;
	roughness = _roughness;
	uvScale = _uvScale;
	uvOffset = _uvOffset;
}

// Deconstructor, currently empty
Material::~Material()
{
}

// Getters
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertexShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }
float Material::GetRoughness() { return roughness; }
float Material::GetUvScale() { return uvScale; }
DirectX::XMFLOAT2 Material::GetUvOffset() { return uvOffset; }

// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint) { colorTint = _colorTint; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader) { vertexShader = _vertexShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader) { pixelShader = _pixelShader; }
void Material::SetRoughness(float _roughness) { roughness = _roughness; }
void Material::SetUvScale(float _uvScale) {	uvScale = _uvScale; }
void Material::SetUvOffset(DirectX::XMFLOAT2 _uvOffset) { uvOffset = _uvOffset; }

/// <summary>
/// Adds a texture to the shader resource view map
/// </summary>
/// <param name="name">The name of the shader resource</param>
/// <param name="texture">The ComPtr to the texture</param>
void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture)
{
	textureSRVs.insert({ name, texture });
}

/// <summary>
/// Adds a sampler state to the sampler state map
/// </summary>
/// <param name="name">The name of the shader resource</param>
/// <param name="state">The ComPtr to the sampler state</param>
void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> state)
{
	samplers.insert({ name, state });
}

/// <summary>
/// Sets the Shader Resource View and Sampler State in the pixel shader
/// </summary>
void Material::SetMaps()
{
	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }
	pixelShader->CopyAllBufferData();
}
