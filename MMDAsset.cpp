#include "pch.h"
#include "MyUtil.h"
#include "MMD.h"
#include "MMDAsset.h"
#include "memstream.h"



uint32_t MMDAsset::GetVerticesCount() const
{
	ASSERT(mScene);
	return (uint32_t)mScene->GetVerticesCount();
}

bool MMDAsset::LoadPMDFromFile(const std::string& filePath)
{
	ASSERT(mScene);
	std::ifstream ifs(filePath, (std::ios::binary | std::ios::in));
	if (!ifs.is_open())
	{
		TRACE("エラー: ファイルを開けませんでした: %s\n", filePath.c_str());
		ASSERT(0);
		return false;
	}
	ifs.seekg(0, std::ios::end);
	auto fileLen = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	std::vector<uint8_t> data(fileLen, 0x00);
	if (ifs.bad())
	{
		TRACE("%s\n", filePath.c_str());
		DBGBREAK();
		return false;
	}
	ifs.read((char*)data.data(), fileLen);
	ifs.seekg(0, std::ios::beg);
	ifs.close();
	IMemStream ms(data);
	return LoadPMDFromStream(ms);
}

bool MMDAsset::LoadPMDFromStream(std::istream& is)
{
	ASSERT(mScene);
	return mScene->LoadPMDStream(is);
}

bool MMDAsset::LoadVMDFromFile(const std::string& filePath)
{
	ASSERT(mScene);
	std::ifstream ifs(filePath, (std::ios::binary | std::ios::in));
	if (!ifs.is_open())
	{
		TRACE("エラー: VMDファイルを開けませんでした: %s\n", filePath.c_str());
		return false;
	}
	ifs.seekg(0, std::ios::end);
	auto fileLen = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	std::vector<uint8_t> data(fileLen, 0x00);
	if (ifs.bad())
	{
		TRACE("%s\n", filePath.c_str());
		DBGBREAK();
		return false;
	}
	ifs.read((char*)data.data(), fileLen);
	ifs.seekg(0, std::ios::beg);
	ifs.close();
	IMemStream ms(data);
	return LoadVMDFromStream(ms);
}

bool MMDAsset::LoadVMDFromStream(std::istream& is)
{
	ASSERT(mScene);
	return mScene->LoadVMDStream(is);
}

void MMDAsset::Prepare()
{
	ASSERT(mScene);
	mScene->Prepare();
}

bool MMDAsset::Init()
{
	mScene = _NEW mmd::MMDScene;
	return false;
}

void MMDAsset::Term()
{
	if (mScene)
	{
		delete mScene;
		mScene = nullptr;
	}
}

// 指定されたフレーム時間に基づいてモデルのアニメーションを更新します。
// この関数はボーンの位置と回転を補間し、モデルの現在のポーズを設定します。
// frameTime: 現在のフレーム時間。
void MMDAsset::Update(float frameTime)
{
	ASSERT(mScene);
	mScene->Update(frameTime);
}

//void MMDAsset::CalcBbox()
//{
//	ASSERT(mScene);
//	mScene->CalcBbox();
//}

uint32_t MMDAsset::GetMaterialCount() const
{
	ASSERT(mScene);
	return (uint32_t)mScene->GetMaterialCount();
}

MMDAsset::Material MMDAsset::GetMaterial(uint32_t materialIdx) const
{
	ASSERT(mScene);
	auto pmdMat = mScene->GetMaterial(materialIdx);
	Material mat;
	mat.diffuse = vec4(pmdMat.diffuse[0], pmdMat.diffuse[1], pmdMat.diffuse[2], pmdMat.alpha);
	mat.ambient = vec3(pmdMat.ambient[0], pmdMat.ambient[1], pmdMat.ambient[2]);
	mat.specular = vec4(pmdMat.specular[0], pmdMat.specular[1], pmdMat.specular[2], pmdMat.specularity);
	mat.texname = pmdMat.texture_filename;
	mat.tex2d = mScene->GetTexture(materialIdx);
	mat.vertexCount = pmdMat.vertex_count;
	return mat;
}

Texture2D* MMDAsset::GetTexture(uint32_t materialIdx) const
{
	ASSERT(mScene);
	return mScene->GetTexture(materialIdx);
}

void MMDAsset::SetTexture(uint32_t materialIdx, Texture2D* tex)
{
	ASSERT(mScene);
	mScene->SetTexture(materialIdx, tex);
}

void MMDAsset::DrawBone(std::vector<VertexPC>* pc)
{
	ASSERT(mScene);
	mScene->DrawBone(pc);
}

void MMDAsset::DrawBoneBbox(std::vector<VertexPC>* pc)
{
	ASSERT(mScene);
	mScene->DrawBoneBbox(pc);
}

// 特定のメッシュを描画するためのOpenGL固定機能パイプラインの実装です。
// この関数は、OpenGLがすでに初期化され、有効なレンダリングコンテキストがアクティブであると仮定します。
// テクスチャのロードとOpenGLテクスチャIDの管理は、このクラスの外部で行われる必要があります。
// _meshIndex: 描画するメッシュのインデックス。
// _pConstantBufferData: 定数バッファデータ (例: 変換行列) への汎用ポインタ。
void MMDAsset::DrawMesh(uint32_t materialIdx, std::vector<VertexPNT>* vert)
{
	if (mScene)
	{
		mScene->DrawMesh(materialIdx, vert);
	}
	return;

	// 無効なメッシュインデックスのチェック
	if (materialIdx < GetMaterialCount())
	{
		return;
	}

	//const Mesh& currentMesh = meshes[_meshIndex];

	//// マテリアルプロパティの設定
	//// OpenGLのGL_AMBIENT_AND_DIFFUSEは、環境光と拡散光の両方に同じ色を使用します。
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glm::value_ptr(currentMesh.diffuseColor));
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(currentMesh.specularColor));
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, currentMesh.specularity);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(glm::vec4(currentMesh.ambientColor, 1.0f))); // ambientColorはvec3なのでvec4に変換


	// 頂点属性を有効化
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	//if (currentMesh.hasTexture)
	//{
	//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//}

	// 頂点データへのポインタを設定
	//glVertexPointer(3, GL_FLOAT, sizeof(Vertex), glm::value_ptr(vertices[0].position));
	//glNormalPointer(GL_FLOAT, sizeof(Vertex), glm::value_ptr(vertices[0].normal));
	//if (currentMesh.hasTexture)
	//{
	//	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), glm::value_ptr(vertices[0].uv));
	//}

	//int firstVertexIndex = 0;
	//for (int i = 0; i < _meshIndex; ++i)
	//{
	//	firstVertexIndex += meshes[i].vertexNum;
	//}

	//// インデックスバッファを使用して描画
	//glDrawElements(GL_TRIANGLES, currentMesh.vertexNum, GL_UNSIGNED_INT, &surfaces[firstVertexIndex].vertexIndex);

	//// 頂点属性を無効化
	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//if (currentMesh.hasTexture)
	//{
	//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//}

	// デバッグ出力
	//TRACEA("MMDAsset::drawMeshがメッシュインデックス: %d (頂点数: %u) で呼び出されました。\n", _meshIndex, currentMesh.vertexNum);
}

// MMDAssetが保持するリソースをクリーンアップします。
// これは主に、内部のベクターをクリアしてメモリを解放することを含みます。
// レンダリングアプリケーションでは、ここには
// グラフィックスAPI固有のリソース (例: VBO、テクスチャID、シェーダープログラム) を
// 解放するための呼び出しも追加されます。
//void MMDAsset::end()
//{
//	// 割り当てられたメモリを解放するために、すべてのベクターをクリアします。
//	vertices.clear();
//	surfaces.clear();
//	texturePaths.clear();
//	materials.clear();
//	bones.clear();
//	meshes.clear();
//
//	// グラフィックスアプリケーションでは、ここに以下を削除/解放する呼び出しを追加します。
//	// - OpenGL VBO、VAO、EBO (glDeleteBuffers, glDeleteVertexArrays)
//	// - Vulkan Buffer、Image、Image View、Sampler
//	// - DirectX Buffer、Shader、Resource、Sampler State
//	// など。
//}

