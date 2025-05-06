#include "common.inc"

in vec3 vertPos;
in vec3 vertPosition_World;
in vec3 vertNormal;
in vec4 vertColor;
in vec2 vertTex;
in vec3 vertTangent;

//layout(location = 0) out vec4 FragColor;
out vec4 FragColor;

void main()
{
	//vec4(u_Material.AmbientColor,1);
	//vec4(Ads(0,vec4(vertPos,1),vertNormal),1);
	if (u_EnableTexture!=0)
	{
		vec4 texc = texture(u_TextureUnit, vertTex);
		FragColor = calculateLighting(vertPos, vertNormal, texc);
	}
	else
	{
		FragColor = calculateLighting(vertPos, vertNormal, vec4(0.0));
	}

#if 0
	// �}�e���A���p�����[�^
	float3 ambientColor = float3(0.1, 0.1, 0.1);
	float3 diffuseColor = float3(0.8, 0.8, 0.8);
	float3 specularColor = float3(1.0, 1.0, 1.0);
	float shininess = 32.0;
	
	// �J�����ʒu
	float3 CameraPosition = float3(0.0, 0.0, 0.0);	//-View[3]

	// �������
	float3 lightDirection = normalize(float3(1.0, 1.0, 1.0));
	float3 lightColor = float3(1.0, 1.0, 1.0);

	// �@���x�N�g��
	float3 normal = normalize(input.normal);

	// ����
	float3 ambient = ambientColor;

	// �g�U���ˌ�
	float diffuseIntensity = max(dot(lightDirection, normal), 0.0);
	float3 diffuse = (diffuseColor * lightColor * diffuseIntensity) * input.color.rgb; // ���_�J���[�K�p

	// ���ʔ��ˌ�
	float3 reflection = reflect(-lightDirection, normal);
	float3 viewDirection = normalize(CameraPosition - input.position.xyz);
	float specularIntensity = pow(max(dot(reflection, viewDirection), 0.0), shininess);
	float3 specular = specularColor * lightColor * specularIntensity;

	float3 finalColor = ambient + diffuse + specular;
//	finalColor = input.color.rgb;
	return myTexture.Sample(mySampler, input.uv);
	return float4(finalColor, input.color.a);
#endif
}
