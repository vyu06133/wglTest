// MMDAsset.h
#pragma once

#include "framework.h"
#include "Vertex.h"
#include "Texture2D.h"
namespace mmd
{
	class MMDScene;
};

class MMDAsset
{
private:
	mmd::MMDScene* mScene = nullptr;	// mmdシーンインスタンス
public:
	struct Material
	{
		vec4 diffuse;	// r, g, b, Alpha
		vec4 specular;	// r, g, b, Shiness
		vec3 ambient;	// r, g, b
		uint32_t vertexCount;
		std::string texname;
		Texture2D* tex2d;
	};
	/**
	 * @brief PMXモデルデータをファイルからロードしてMMDAssetを初期化します。
	 * このメソッドは、loadPMXとModel::initの機能をカプセル化します。
	 *
	 * @param _filePath PMXモデルファイルへのワイド文字列パス。
	 * @return モデルが正常にロードされた場合はtrue、そうでない場合はfalse。
	 */
	bool Init();
	void Term();

	uint32_t GetVerticesCount() const;
	bool LoadPMDFromFile(const std::string& filePath);
	bool LoadPMDFromStream(std::istream& is);
	bool LoadVMDFromFile(const std::string& filePath);
	bool LoadVMDFromStream(std::istream& is);
	
	void Prepare();

	void Update(float frameTime);
	float GetFrameTime() const;

	uint32_t GetMaterialCount() const;
	Material GetMaterial(uint32_t materialIdx)const;
	Texture2D* GetTexture(uint32_t materialIdx) const;
	void SetTexture(uint32_t materialIdx, Texture2D* tex);
	void DrawBone(std::vector<VertexPC>* pc);
	void DrawBoneBbox(std::vector<VertexPC>* pc);
	void DrawMesh(uint32_t materialIdx, std::vector<VertexPNT>* vert);

};
