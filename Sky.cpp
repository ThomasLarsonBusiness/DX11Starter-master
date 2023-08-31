#include "Sky.h"

/// <summary>
/// Constructor for the skybox
/// </summary>
/// <param name="_mesh">The geometry for the skybox</param>
/// <param name="_samplerState">The sampler state for sampling options</param>
/// <param name="device">Reference to the device</param>
/// <param name="_vertexShader">The vertex shader for the skybox</param>
/// <param name="_pixelShader">The pixel shader for the skybox</param>
/// <param name="texture">The DDS texture for the skybox</param>
Sky::Sky(std::shared_ptr<Mesh> _mesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState, Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture)
{
	// Sets appropriate variables
	samplerState = _samplerState;
	skyTexture = texture;
	mesh = _mesh;
	vertexShader = _vertexShader;
	pixelShader = _pixelShader;

	// Creates a rasterizer state
	D3D11_RASTERIZER_DESC rastDesc= {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rastDesc, rasterizerState.GetAddressOf());

	// Creates a depth stencil
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthDesc, depthStencil.GetAddressOf());
}

// Will stay empty for now
Sky::~Sky()
{
}

/// <summary>
/// Draws the skybox. Should be done after all other objects are drawn.
/// </summary>
/// <param name="context"></param>
/// <param name="camera"></param>
void Sky::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	// Sets the rasterizer and depth stencil states
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthStencil.Get(), 0);

	// Sets the shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	// Sets shader variables
	vertexShader->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projectionMatrix", camera->GetProjectionMatrix());
	vertexShader->CopyAllBufferData();

	pixelShader->SetShaderResourceView("skybox", skyTexture);
	pixelShader->SetSamplerState("samplerState", samplerState);
	pixelShader->CopyAllBufferData();

	// Draw the skybox
	// Sets stride and offset
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Sets the Vertex and Index Buffers
	context->IASetVertexBuffers(0, 1, mesh->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	// Draws the mesh to the screen
	context->DrawIndexed(
		mesh->GetIndexCount(),
		0,
		0);

	// Reset Rasterizer and Depth Stencil States
	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(nullptr, 0);
}
