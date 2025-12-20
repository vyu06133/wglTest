#include "common.glsl"

//in vec3 a_position;
//in vec3 a_normal;
//in vec4 a_color;
//in vec2 a_texcoord;
//in vec3 a_tangent;
//in uvec4 a_bone;
//in vec4 a_weight;
in vec3 a_position;
in vec3 a_normal;
in vec4 a_color;
in vec2 a_texcoord;
in vec3 a_tangent;
in uvec4 a_bone;
in vec4 a_weight;


out vec3 v_pos;
out vec3 v_position_World;
out vec3 v_normal;
out vec4 v_color;
out vec2 v_texcoord;
out vec3 v_tangent;


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
		}
		else
		{
			o.position = di.position;
			o.normal = di.normal;
			o.color = di.color;
			o.tangent = di.tangent;
		}
o.position = di.position;//test:
o.color = di.weights;//test:
		return o;
	}
}

DeformOut Bypass(DeformIn di)
{
	DeformOut o;
	o.position = di.position;
	o.normal = di.normal;
	o.color = di.color;
	o.tangent = di.tangent;
o.color = di.color*0.00001+di.weights;//optimize off:
	return o;
}

void main()
{
	DeformIn di;
	di.position = a_position;
	di.normal = a_normal;
	di.color = a_color;
	di.tangent = a_tangent;
	di.bones = a_bone;
	di.weights = a_weight;
	DeformOut o;
//	if (u_EnableDeform != 0)
//	{
//		o = Skinning(di);
//	}
//	else
	{
		o = Bypass(di);
	}
	
	v_pos = (u_Constants.worldViewProj * vec4(o.position, 1.0f)).xyz;
	v_normal = o.normal;
	v_color = o.color;
	v_texcoord = a_texcoord;
	v_tangent = o.tangent;
	gl_Position = u_Constants.worldViewProj * vec4(o.position, 1.0);
}