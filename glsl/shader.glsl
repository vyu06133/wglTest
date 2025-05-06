#define MAX_PALETTE_SIZE 96
#define MAX_LIGHT 3

// Vertex Shader (VertexShader.hlsl)
uniform struct MatrixPalette
{
	mat4 clusterDeformations[MAX_PALETTE_SIZE];
};


uniform struct Constants
{
	mat4 worldViewProj;
	mat4 world;
	mat3 worldInverseTranspose;
	mat4 view;
	mat4 projection;

	vec3 lightPosition[MAX_LIGHT];
	vec4 lightDiffuseColor[MAX_LIGHT];
	vec4 lightSpecularColor[MAX_LIGHT];
};

uniform struct Materials
{
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
};


layout (location = 0) in vec3 cpu_position;
layout (location = 1) in vec3 cpu_normal;
layout (location = 2) in vec4 cpu_color;
layout (location = 3) in vec2 cpu_tex;
layout (location = 4) in vec3 cpu_tangent;
layout (location = 5) in uint4 cpu_bones;
layout (location = 6) in vec4 cpu_weights;

struct PixelInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
};



