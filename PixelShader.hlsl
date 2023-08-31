#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float3 cameraPosition;
	float roughness;
	float3 cameraPos;
	float3 ambientLight;
	float uvScale;
	float2 uvOffset;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;
}

//Texture2D SurfaceTexture  : register(t0);		For Non-PBR Lighting
Texture2D AlbedoTexture		: register(t0);
Texture2D NormalMap			: register(t1);
Texture2D RoughnessMap		: register(t2);
Texture2D MetalnessMap		: register(t3);
SamplerState BasicSampler	: register(s0);


float3 Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

// Hear until CalculateDirectionalLight() was made by Chris Casciolli
// CONSTANTS ===================

// Make sure to place these at the top of your shader(s) or shader include file
// - You don't necessarily have to keep all the comments; they're here for your reference

// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;

// PBR FUNCTIONS ================

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount based on energy conservation
//
// diffuse - Diffuse amount
// specular - Specular color (including light color)
// metalness - surface metalness amount
//
// Metals should have an albedo of (0,0,0)...mostly
// See slide 65: http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
float3 DiffuseEnergyConserve(float3 diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

// GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1; MIN_ROUGHNESS helps here

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n (full specular color)
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2, roughness remapped to (r+1)/2
//
// n - Normal
// v - View vector
//
// G(l,v)
float GeometricShadowing(float3 n, float3 v, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}

// Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
//
// D() - Spec Dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor)
{
	// Other vectors
	float3 h = normalize(v + l);

	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, roughness) * GeometricShadowing(n, l, roughness);

	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}

// Does the light calculations for directional lights
float3 CalculateDirectionalLight(Light light, VertexToPixel inputData, float roughnessValue, float metalnessValue, float3 surfaceValue)
{
	// Normalized direction to the light
	float3 dirToLight = normalize(-light.Direction);

	// Calculate Diffuse Amount
	float diffuseAmount = saturate(dot(inputData.normal, dirToLight));
	
	// Specular for nonPBR light
	/*
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float specular;
	if (specExponent > 0.05f)
	{
		float3 viewVectorToCam = normalize(cameraPosition - inputData.worldPosition);
		float3 reflVector = reflect(-dirToLight, inputData.normal);
		specular = pow(saturate(dot(reflVector, viewVectorToCam)), specExponent);
	}
	*/

	// Determines the specular color
	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceValue.rgb, metalnessValue);

	// Does the Cook-Torence Calculation
	float3 specularValue = MicrofacetBRDF(inputData.normal, dirToLight, normalize(cameraPosition - inputData.worldPosition), roughnessValue, specularColor);

	// Calculate diffuse with energy conservation
	float3 balancedDiff = DiffuseEnergyConserve(diffuseAmount, specularValue, metalnessValue);
	
	// Calculate the final color
	//float3 lightColor = ((diffuseAmount * light.Color * colorTint) + (ambientLight * colorTint) + specular) * light.Intensity;
	float3 lightColor = (balancedDiff * surfaceValue + specularValue) * light.Intensity * light.Color;
	return lightColor;
}

// Does the light calculations for point lights
float3 CalculatePointLight(Light light, VertexToPixel inputData, float roughnessValue, float metalnessValue, float3 surfaceValue)
{
	// Normalized direction to the light
	float3 dirToLight = normalize(light.Position - inputData.worldPosition);

	// Calculate Diffuse Amount
	float diffuseAmount = saturate(dot(inputData.normal, dirToLight));

	// Specular for nonPBR light
	/*
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float specular;
	if (specExponent > 0.05f)
	{
		float3 viewVectorToCam = normalize(cameraPosition - inputData.worldPosition);
		float3 reflVector = reflect(-dirToLight, inputData.normal);
		specular = pow(saturate(dot(reflVector, viewVectorToCam)), specExponent);
	}
	*/

	// Determines the specular color
	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceValue.rgb, metalnessValue);

	// Does the Cook-Torence Calculation
	float3 specularValue = MicrofacetBRDF(inputData.normal, dirToLight, normalize(cameraPosition - inputData.worldPosition), roughnessValue, specularColor);

	// Calculate diffuse with energy conservation
	float3 balancedDiff = DiffuseEnergyConserve(diffuseAmount, specularValue, metalnessValue);

	// Calculate the final color
	float3 lightColor = (balancedDiff * surfaceValue + specularValue) * light.Intensity * light.Color * Attenuate(light, inputData.worldPosition);
	return lightColor;
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
	// Normalize the normals
	input.normal = normalize(input.normal);

	// Scales/Shifts the uvs
	input.uv = (input.uv + uvOffset) * uvScale;

	// Sets texture colors
	float3 surfaceColor = pow(AlbedoTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);

	// Tints the surface color with material surface
	surfaceColor = surfaceColor * colorTint;

	// Unpacks the normals
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;

	// Creates a TBN matrix
	float3 N = input.normal;
	float3 T = normalize(input.tangent);
	T = normalize(T - N * dot(T, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Transform the unpacked normal
	input.normal = mul(unpackedNormal, TBN);

	// Samples the roughness map
	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;

	// Samples the metalness map
	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	// Adds lights values to the object
	//float3 finalColor = surfaceColor + CalculateDirectionalLight(directionalLight1, input) + CalculateDirectionalLight(directionalLight2, input) + CalculateDirectionalLight(directionalLight3, input);
	//finalColor += CalculatePointLight(pointLight1, input) + CalculatePointLight(pointLight2, input);

	float3 finalColor = CalculateDirectionalLight(directionalLight1, input, roughness, metalness, surfaceColor) + CalculateDirectionalLight(directionalLight2, input, roughness, metalness, surfaceColor) +
		CalculateDirectionalLight(directionalLight3, input, roughness, metalness, surfaceColor);
	finalColor = finalColor + CalculatePointLight(pointLight1, input, roughness, metalness, surfaceColor) + CalculatePointLight(pointLight2, input, roughness, metalness, surfaceColor);

	return float4(pow(finalColor, 1.0f/2.2f), 1);
}