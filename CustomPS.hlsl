#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float totalTime;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 total = 0.0f;
	float v = (input.uv.y * -2 + 1) * 10.0f;
	float f = 3.0f * 3.14159f * 2.0f;
	float s = sin(input.uv.x * f + totalTime * 4.0f + sin(totalTime)) * 5.0f;

	float dist = 1.0f - saturate(pow(abs(v - s), 2.0f));
	total += float3(cos(totalTime), sin(totalTime), 0.0f) * dist * 15.0f;

	return float4(colorTint * total, 1.0f);
}