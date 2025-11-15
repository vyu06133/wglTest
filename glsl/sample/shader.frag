#include "type.hlsli"

Texture2D myTexture : register(t0);
SamplerState mySampler : register(s0);
//Texture2D myTexture;
//SamplerState mySampler;

float4 PSMain(PixelInput input) : SV_TARGET
{
  // マテリアルパラメータ
	float3 ambientColor = float3(0.1, 0.1, 0.1);
	float3 diffuseColor = float3(0.8, 0.8, 0.8);
	float3 specularColor = float3(1.0, 1.0, 1.0);
	float shininess = 32.0;
	
  // カメラ位置
	float3 CameraPosition = float3(0.0, 0.0, 0.0);	//-View[3]

  // 光源情報
	float3 lightDirection = normalize(float3(1.0, 1.0, 1.0));
	float3 lightColor = float3(1.0, 1.0, 1.0);

  // 法線ベクトル
	float3 normal = normalize(input.normal);

  // 環境光
	float3 ambient = ambientColor;

  // 拡散反射光
	float diffuseIntensity = max(dot(lightDirection, normal), 0.0);
	float3 diffuse = (diffuseColor * lightColor * diffuseIntensity) * input.color.rgb; // 頂点カラー適用

  // 鏡面反射光
	float3 reflection = reflect(-lightDirection, normal);
	float3 viewDirection = normalize(CameraPosition - input.position.xyz);
	float specularIntensity = pow(max(dot(reflection, viewDirection), 0.0), shininess);
	float3 specular = specularColor * lightColor * specularIntensity;

	float3 finalColor = ambient + diffuse + specular;
//	finalColor = input.color.rgb;
	return myTexture.Sample(mySampler, input.uv);
	return float4(finalColor, input.color.a);
}
