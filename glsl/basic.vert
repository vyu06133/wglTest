#include "common.inc"

layout (location = 0) in vec3 cpu_position;
layout (location = 1) in vec3 cpu_normal;
layout (location = 2) in vec4 cpu_color;
layout (location = 3) in vec2 cpu_tex;
layout (location = 4) in vec3 cpu_tangent;
layout (location = 5) in uvec4 cpu_bones;
layout (location = 6) in vec4 cpu_weights;

out vec3 vertPos;
out vec3 vertPosition_World;
out vec3 vertNormal;
out vec4 vertColor;
out vec2 vertTex;
out vec3 vertTangent;


struct DeformIn
{
	vec3 position;
	vec3 normal;
	vec4 color;
	vec3 tangent;
	uvec4 bones;
	vec4 weights;
};

struct DeformOut
{
	vec3 position;
	vec3 normal;
	vec4 color;
	vec3 tangent;
};

DeformOut Skinning(DeformIn di)
{
	if((di.weights[0] + di.weights[1] + di.weights[2] + di.weights[3]) == 0)
	{
		return DeformOut(di.position, di.normal, di.color, di.tangent);
	}
	else
	{
		DeformOut o = DeformOut(vec3(0,0,0), vec3(0,0,0), di.color, vec3(0,0,0));
		float sum = 0;
	
		for (int j = 0; j < 4; j++) // BONES_PER_VERTEX = 4
		{
			float weight = di.weights[j];
			if (weight != 0)
			{
				sum += weight;
				
				uint b = di.bones[j];
				mat4 positionMatrix = u_Palette.Matrices[b];
				mat3 normalMatrix = transpose(inverse(mat3(positionMatrix))); // ³Šm‚È–@ü•ÏŠ·—p
				
				o.position += (positionMatrix * vec4(di.position, 1.0f)).xyz * weight;
				
				o.normal += normalMatrix * di.normal * weight;
				o.tangent += normalMatrix * di.tangent * weight;
			}
		}
		if (sum != 0)
		{
			o.color = di.color;
			o.normal = normalize(o.normal);
			o.tangent = normalize(o.tangent);
			return o;
		}
		else
		{
			o.position = di.position;
			o.normal = di.normal;
			o.tangent = di.tangent;
			return o;
		}
	}
}

DeformOut Bypass(DeformIn i)
{
	DeformOut o;
	o.position = i.position;
	o.normal = i.normal;
	o.tangent = i.tangent;
	return o;
}

void main()
{
	DeformIn di;
	di.position = cpu_position;
	di.normal = cpu_normal;
	di.color = cpu_color;
	di.tangent = cpu_tangent;
	di.bones = cpu_bones;
	di.weights = cpu_weights;
	DeformOut o;
	if (u_EnableDeform != 0)
	{
		o = Skinning(di);
		vertColor = o.color;
		vertTex = cpu_tex;
	}
	else
	{
		o = Bypass(di);
		vertColor = o.color;
		vertTex = cpu_tex;
	}
	
	vertPos = (u_Constants.worldViewProj * vec4(o.position, 1.0f)).xyz;
	vertNormal = o.normal;
	vertTex = cpu_tex;
	vertTangent = o.tangent;
	gl_Position = u_Constants.worldViewProj * vec4(o.position, 1.0);
//	vertPosition_World = (u_Constants.world * vec4(cpu_position, 1.0)).xyz;
//	vertPos = (u_Constants.world * u_Constants.view * vec4(cpu_position, 1.0)).xyz;
//	vertPos = (u_Constants.view * vec4(cpu_position, 1.0)).xyz;
//	vertNormal = normalize(cpu_normal);
//	vertTex = cpu_tex;
//	vertColor = cpu_color;
	
/*
	vec3 eyeNorm = normalize( u_Constants.worldInverseTranspose * cpu_Normal);
	vec4 eyePosition = ModelViewMatrix * vec4(VertexPosition,1.0);

	// Evaluate the lighting equation, for each light
	vertColor = vec3(0.0);
	for( int i = 0; i < MAX_LIGHT; i++ )
		vertColor += Ads( i, eyePosition, eyeNorm );
*/
}