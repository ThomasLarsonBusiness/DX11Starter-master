struct VertexShaderInput 
{
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
	float3 tangent			: TANGENT;
};

struct VertexToPixel
{
	float4 position			: SV_POSITION;
	float3 sampleDir		: DIRECTION;
};

cbuffer ExternalData : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
}

VertexToPixel main(VertexShaderInput input)
{
	// Variable for 
	VertexToPixel output;

	// Remove translation from the view matrix
	matrix copyView = viewMatrix;
	copyView._14 = 0;
	copyView._24 = 0;
	copyView._34 = 0;

	// Apply view and projection matrix to position
	matrix viewProj = mul(projectionMatrix, copyView);
	output.position = mul(viewProj, float4(input.localPosition, 1.0f));

	// Set output depth to exactly 1.0f
	output.position.z = output.position.w;

	// Set sample direction
	output.sampleDir = input.localPosition;

	return output;
}