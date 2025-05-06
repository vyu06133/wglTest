#version 330
#include "shader.glsl"
struct MatrixPaletteBuffer
{
	mat4 clusterDeformations[MAX_PALETTE_SIZE];
};

VertexInput Skinning(VertexInput input)
{
	VertexInput output;
	float3 skinnedPos = float3(0, 0, 0);
	float3 skinnedNormal = float3(0, 0, 0);
	float s = 0;
	
	for (int j = 0; j < 4; j++) // BONES_PER_VERTEX = 4
	{
		float w = input.boneWeights[j];
		if (w == 0.0f)
			break;
		s += w;

		int b = input.boneIDs[j];
        
		skinnedPos += mul(float4(input.position, 1.0f), clusterDeformations[b]).xyz * w;
        
		float3x3 normalMatrix = (float3x3) clusterDeformations[b]; // ã3~3‚¾‚¯Žg—p
		skinnedNormal += mul(input.normal, normalMatrix) * w;
	}
	if (s == 0)
	{
		return input;
	}
	else
	{
		output.position = skinnedPos;
		output.normal = normalize(skinnedNormal);
		output.uv = input.uv;
		output.color = input.color;

		return output;
	}
}

PixelInput main(VertexInput input)
{
	PixelInput output;
	VertexInput skinned = Skinning(input);
//	VertexInput skinned = input;
	output.position = mul(float4(skinned.position, 1.0f), WorldViewProjection);
	output.normal = skinned.normal;
	output.color = skinned.color;
	output.uv = skinned.uv;
//	output.color = World[0].rgba * 0.5;
//	output.color = input.boneWeights;
//	output.color = float4(skinned.normal, 1);
//	output.color = float4(0, 1, 0, .5);
//	output.color = float4(diffuseColor, 1);
	return output;
}
