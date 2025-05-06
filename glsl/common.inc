#version 400 core
/*
430ならばUniform blockのレイアウトでbinding=NとしてNでバインドできるはずだがC++が未対応なので400とまり
*/
#define MAX_PALETTE_SIZE	96
#define MAX_LIGHT			8

layout(std140) uniform Constants
{
	mat4  worldViewProj;
	mat4  world;
	mat4  view;
	mat4  projection;
	vec3  eye;
	mat3  Normal;
} u_Constants;

layout(std140) uniform Palette
{
	mat4  Matrices[MAX_PALETTE_SIZE];
} u_Palette;

layout(std140) uniform LightInfo
{
	vec4 Position_EyeCoord[MAX_LIGHT]; // Light position in eye coords. w成分==0.0ならばディレクショナルライト
	float Range[MAX_LIGHT];
	vec3 Intensity[MAX_LIGHT];
} u_LightInfo;

layout(std140) uniform Material
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
	vec3 EmmisiveColor;
	vec3 SpecularColor;
	float Shininess;
} u_Material;

uniform sampler2D u_TextureUnit;//glActiveTextureするもの、GL_TEXTURE0=0,GL_TEXTURE5=5になる
uniform int u_EnableTexture;
uniform int u_EnableDeform;
uniform int u_EnableFog;
uniform int u_EnableLighting;

vec4 calculateLighting(vec3 position, vec3 normal, vec4 textureColor)
{
//return vec4(0,1,0,1);//test:
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(u_Constants.eye - position);
//return vec4(viewDir,1);//test:

	vec4 finalColor = vec4(0.0);

	for (int i = 0; i < 1/*MAX_LIGHT*/; ++i)
	{
		vec3 lightDir;
		vec3 lightPosInView;//ビュー空間での座標

		// 光源の種類で計算を分岐
		lightPosInView = (u_Constants.view * vec4(u_LightInfo.Position_EyeCoord[i].xyz, 1.0)).xyz; 
//return vec4(lightPosInView,1);//test:
//bug:アヤシイ
		if (u_LightInfo.Position_EyeCoord[i].w == 0.0)
		{
			// ディレクショナルライト
			lightDir = normalize(-lightPosInView);
		}
		else
		{
			// ポイントライト
			lightDir = normalize(lightPosInView - position);
		}

		// Ambient
		vec4 ambient = vec4(u_Material.AmbientColor, 1.0) * textureColor;

		// Diffuse
		float diff = max(dot(norm, lightDir), 0.0);
//return vec4(diff,diff,diff,1);
//diff=1.0f;//test:
		vec4 diffuse = vec4(u_Material.DiffuseColor, 1.0) * diff * textureColor;
//diffuse=textureColor;//test:
		// Specular
		vec3 reflectDir = reflect(-lightDir, norm);
		float specRaw = dot(viewDir, reflectDir);
//specRaw=1.0f;//test:
		float spec = pow(max(specRaw, 0.0), u_Material.Shininess);
		vec4 specular = vec4(u_Material.SpecularColor, 1.0) * spec;

		finalColor += ambient + diffuse + specular;
	}

//finalColor = vec4(0,1,0,1);//test:
//finalColor = textureColor;
	// エミッション（他の陰影に影響されない）
	finalColor += vec4(u_Material.EmmisiveColor, 1.0);
//finalColor *= textureColor;
	finalColor.a = 1.0;
	return finalColor * textureColor;
}
