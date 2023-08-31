#include "ShaderIncludes.hlsli"

// Constant Buffer
// Allows us to store data on the GPU
cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix worldInvTranspose;
	matrix view;
	matrix projection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
	matrix wvp = mul(mul(projection, view), world);
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	output.uv = input.uv;
	
	// Translating Normals
	output.normal = mul((float3x3)worldInvTranspose, input.normal);
	output.tangent = mul((float3x3)worldInvTranspose, input.tangent);
	output.worldPosition = mul(world, float4(input.localPosition, 1)).xyz;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}