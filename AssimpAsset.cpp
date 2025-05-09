#include "pch.h"
#include <assimp/Importer.hpp>
#include "AssimpAsset.h"

AssimpAsset::AssimpAsset()
{
}

AssimpAsset::~AssimpAsset()
{
}

bool AssimpAsset::LoadAsset(const std::string& Filename)
{
	mScene = mImporter.ReadFile(Filename.c_str(),
		aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!mScene || mScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !mScene->mRootNode)
	{
		TRACE("Assimp Error: %s\n", mImporter.GetErrorString() );
		return false;
	}
	mAnimation = mScene->mAnimations[0];
	ParseBoneHierarchy(mScene->mRootNode, nullptr);

	return true;
}

bool AssimpAsset::ParseBoneHierarchy(aiNode* node, NodeTree::Item* parent)
{
	std::string name(node->mName.C_Str());
	NodeInfo ninf(node);
	ninf.relativeParent = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

	auto ins = mNodeTree.Insert(ninf, parent);
std::string indent; for (auto t = 0u; t < ins->depth; t++)indent += " ";
TRACE("%s\"%s\"\n", indent.c_str(), name.c_str());
	mNodeMap[name] = ins;

	for (auto i = 0u; i < node->mNumChildren; i++)
	{
		ParseBoneHierarchy(node->mChildren[i], ins);
	}
	return true;
}

bool AssimpAsset::ParseMesh()
{
	for (unsigned int m = 0; m < mScene->mNumMeshes; ++m)
	{
		aiMesh* mesh = mScene->mMeshes[m];

		// 頂点とインデックス
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			aiVector3D pos = mesh->mVertices[i];
			aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 0, 0);
			// UVなどもここで取得可能
		}

		// ボーンとスキンウェイト
		for (unsigned int b = 0; b < mesh->mNumBones; ++b)
		{
			aiBone* bone = mesh->mBones[b];
			std::string boneName = bone->mName.C_Str();

			// 各頂点への影響
			for (unsigned int w = 0; w < bone->mNumWeights; ++w)
			{
				unsigned int vertexId = bone->mWeights[w].mVertexId;
				float weight = bone->mWeights[w].mWeight;

				// vertexId番の頂点に対してboneNameがweightの影響
			}
		}
		return false;
	}
	return true;
}

const aiNodeAnim* AssimpAsset::FindNodeAnim(const std::string nodeName)
{
	ASSERT(mAnimation);
	for (auto i = 0u; i < mAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* nodeAnim = mAnimation->mChannels[i];
		if (nodeAnim)
		{
			std::string name(nodeAnim->mNodeName.C_Str());

			if (name == nodeName)
			{
				return nodeAnim;
			}
		}
	}

	return nullptr;
}

uint32_t AssimpAsset::FindScaling(const aiNodeAnim* nodeAnim, float animTime)
{
	ASSERT(nodeAnim->mNumScalingKeys > 0);

	for (auto i = 0u; i < nodeAnim->mNumScalingKeys - 1; i++)
	{
		if (animTime < (float)nodeAnim->mScalingKeys[i + 1].mTime)
		{
			return i;
		}
	}

	ASSERT(0);

	return 0;
}

uint32_t AssimpAsset::FindRotation(const aiNodeAnim* nodeAnim, float animTime)
{
	ASSERT(nodeAnim->mNumRotationKeys > 0);

	for (auto i = 0u; i < nodeAnim->mNumRotationKeys - 1; i++)
	{
		if (animTime < (float)nodeAnim->mRotationKeys[i + 1].mTime)
		{
			return i;
		}
	}

	ASSERT(0);

	return 0;
}

uint32_t AssimpAsset::FindPosition(const aiNodeAnim* nodeAnim, float animTime)
{
	ASSERT(nodeAnim->mNumPositionKeys > 0);

	for (auto i = 0u; i < nodeAnim->mNumPositionKeys - 1; i++)
	{
		if (animTime < (float)nodeAnim->mPositionKeys[i + 1].mTime)
		{
			return i;
		}
	}

	ASSERT(0);

	return 0;
}

vec3 AssimpAsset::CalcInterpolatedScaling(const aiNodeAnim* nodeAnim, float animTime)
{
	if (nodeAnim->mNumScalingKeys <= 1)
	{
		auto out = nodeAnim->mScalingKeys[0].mValue;
		return vec3(out.x, out.y, out.z);
	}

	auto ScalingIndex = FindScaling(nodeAnim, animTime);
	auto NextScalingIndex = (ScalingIndex + 1);
	ASSERT(NextScalingIndex < nodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(nodeAnim->mScalingKeys[NextScalingIndex].mTime - nodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (animTime - (float)nodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	ASSERT(Factor >= 0.0f && Factor <= 1.0f);
	const auto& Start = nodeAnim->mScalingKeys[ScalingIndex].mValue;
	const auto& End = nodeAnim->mScalingKeys[NextScalingIndex].mValue;
	auto o = Start + (End - Start) * Factor;
	return vec3(o.x, o.y, o.z);
}

quat AssimpAsset::CalcInterpolatedRotation(const aiNodeAnim* nodeAnim, float animTime)
{
	if (nodeAnim->mNumRotationKeys <= 1)
	{
		auto out = nodeAnim->mRotationKeys[0].mValue;
		return quat(out.w, out.x, out.y, out.z);
	}

	auto RotationIndex = FindRotation(nodeAnim, animTime);
	auto NextRotationIndex = (RotationIndex + 1);
	ASSERT(NextRotationIndex < nodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(nodeAnim->mRotationKeys[NextRotationIndex].mTime - nodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (animTime - (float)nodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	ASSERT(Factor >= 0.0f && Factor <= 1.0f);
	const auto& startQ = nodeAnim->mRotationKeys[RotationIndex].mValue;
	const auto& endQ = nodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion outQ;
	aiQuaternion::Interpolate(outQ, startQ, endQ, Factor);
	//quat s(startQ.w, startQ.x, startQ.y, startQ.z);
	//quat e(endQ.w, endQ.x, endQ.y, endQ.z);
	//auto s = ToGlmQuat(startQ);
	//auto e = ToGlmQuat(endQ);
	return glm::normalize(ToGlmQuat(outQ));
}

vec3 AssimpAsset::CalcInterpolatedPosition(const aiNodeAnim* nodeAnim, float animTime)
{
	if (nodeAnim->mNumPositionKeys <= 1)
	{
		auto out = nodeAnim->mPositionKeys[0].mValue;
		return vec3(out.x, out.y, out.z);
	}

	auto PositionIndex = FindPosition(nodeAnim, animTime);
	auto NextPositionIndex = (PositionIndex + 1);
	ASSERT(NextPositionIndex < nodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(nodeAnim->mPositionKeys[NextPositionIndex].mTime - nodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (animTime - (float)nodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	ASSERT(Factor >= 0.0f && Factor <= 1.0f);
	const auto& Start = nodeAnim->mPositionKeys[PositionIndex].mValue;
	const auto& End = nodeAnim->mPositionKeys[NextPositionIndex].mValue;
	auto o = Start + (End - Start) * Factor;
	return vec3(o.x, o.y, o.z);
}

mat4 AssimpAsset::CalcLocalTransform(NodeTree::Item* item)
{
	ASSERT(item);
	auto& ni = item->data;
	auto node = ni.node;
	ASSERT(node);
	float animTime = mTime;

	// スケール、回転、位置のアニメーション値
	auto nodeAnim = FindNodeAnim(ni.name);
	if (!nodeAnim)
	{
		return ni.relativeParent;
	}

	auto lclScaling = CalcInterpolatedScaling(nodeAnim, animTime);
	auto lclRotation = CalcInterpolatedRotation(nodeAnim, animTime);
	auto lclTranslation = CalcInterpolatedPosition(nodeAnim, animTime);

	// 各行列の構築
	mat4 T = glm::translate(mat4(1.0f), lclTranslation);
	mat4 R = glm::toMat4(lclRotation);
	mat4 S = glm::scale(mat4(1.0f), lclScaling);

	// トランスフォームの合成
	return T * R * S;  // 列ベクトル形式ならこの順序
}

void AssimpAsset::UpdateLocalTransforms()
{
	for (auto itr = mNodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		ni.localTransform = CalcLocalTransform(itr);
	}
}

void AssimpAsset::UpdateWorldTransforms()
{
	for (auto itr = mNodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		if (itr->parent)
		{
			auto& pni = itr->parent->data;
			ni.worldTransform = pni.worldTransform * ni.localTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
		}
		else
		{
			// 親がいない場合、これはルートノードでありローカルトランスフォームがそのままワールドトランスフォーム
			ni.worldTransform = mRootTransform * ni.localTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
		}
	}
}

void AssimpAsset::Update(float delta)
{
//	mTime += mSecPerFrame;
	mTime += delta*30.0f;
	if (mTime > mStop)
	{
		mTime -= (mStop - mStart);
	}

//	m_sampleAlpha -= delta * MOTION_INTERP_RATE;
//	if (m_sampleAlpha < 0.0f)
//	{
//		m_sampleAlpha = 0.0f;
//	}

//	m_rootTransform = glm::scale(mat4(1.0f), vec3(0.1f));
//	UpdateSamplePoint(m_time);
	UpdateLocalTransforms();
	UpdateWorldTransforms();
//	DeformBoneWeight();//note:シェーダーでデフォームする場合これを呼ばない
}

uint32_t AssimpAsset::GetBonePrimCount() const
{
	return mNodeTree.GetCount() * 2;
}

void AssimpAsset::RenderBone(std::vector<VertexPC>* primitive)
{
	for (auto itr = mNodeTree.Begin(); itr; itr = itr->next)
	{
		if (itr->parent)
		{
			vec4 c(1.0f, 0.0f, 1.0f, 1.0f);
			auto posf = MyMath::GetTranslation(itr->data.worldTransform);
			auto pposf = MyMath::GetTranslation(itr->parent->data.worldTransform);
//posf = MyMath::mt.RandRangeVec(vec3(MyMath::Sin(mTime)), vec3(1.f));
//pposf = MyMath::mt.RandRangeVec(-vec3(1.f), vec3(MyMath::Sin(mTime)));
			primitive->push_back(VertexPC(posf, c));
			primitive->push_back(VertexPC(pposf, c));
		}
	}
}
