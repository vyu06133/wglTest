#pragma once
#include "framework.h"
#include "Vertex.h"
#pragma warning(disable : 4819)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

inline static vec3 ToGlmVec(const aiVector3D& v)
{
	return vec3(v.x, v.y, v.z);
}

inline static mat4 ToGlmMat(const aiMatrix4x4& m)
{
	return glm::transpose(glm::make_mat4(&m.a1));  // Assimp‚Ís—DæAGLM‚Í—ñ—Dæ‚È‚Ì‚Å“]’u
}

inline static quat ToGlmQuat(const aiVector3D& v)
{
	glm::quat qx = glm::angleAxis(v.x, glm::vec3(1, 0, 0));
	glm::quat qy = glm::angleAxis(v.y, glm::vec3(0, 1, 0));
	glm::quat qz = glm::angleAxis(v.z, glm::vec3(0, 0, 1));
	return qz * qy * qx; // XYZ‡
}

inline static quat ToGlmQuat(const aiQuaternion& q)
{
	return glm::quat(q.w, q.x, q.y, q.z);
}

class AssimpAsset
{
public:
	struct NodeInfo
	{
		NodeInfo(const aiNode* src = nullptr) :
			name(src ? src->mName.C_Str() : ""),
			type(),
			flags(0),
			id(0),
			node(src),
			//scalingOffset(1.0f),
			//scalingPivot(0.0f),
			//rotationOffset(0.0f),
			//rotationPivot(0.0f),
			//preRotation(),
			//postRotation(),
			//lclScaling(""),
			//lclRotation(""),
			//lclTranslation(""),
			worldTransform(1.0f),
			localTransform(1.0f)
		{
			if (src)
			{
				relativeParent = glm::make_mat4(&src->mTransformation.a1);
			}
		}
		std::string name;
		std::string type;
		uint32_t flags;
		uint32_t id;
		const aiNode* node;
		mat4 relativeParent;
		vec3 lclScaling;
		quat lclRotation;
		vec3 lclTranslation;
		//vec3 scalingOffset;
		//vec3 scalingPivot;
		//vec3 rotationOffset;
		//vec3 rotationPivot;
		//quat preRotation;
		//quat postRotation;
		//std::string lclScaling;
		//std::string lclRotation;
		//std::string lclTranslation;
		mat4 worldTransform;
		mat4 localTransform;
	};

	using NodeTree = MyUtil::Tree<NodeInfo>;
	NodeTree mNodeTree;
	std::unordered_map<std::string, NodeTree::Item*> mNodeMap;
	Assimp::Importer mImporter;
	const aiScene* mScene = nullptr;
	const aiAnimation* mAnimation = nullptr;
	mat4 mRootTransform = mat4(0.1f);//todo:
	float mTime = 0.0f;
	float mSecPerFrame = 1.0f / 60.0f;
	float mStart = 0.0f;
	float mStop = 0.0f;
	AssimpAsset();
	~AssimpAsset();
	bool LoadAsset(const std::string& Filename);
	bool ParseBoneHierarchy(aiNode* node, NodeTree::Item* parent);
	bool ParseMesh();
	const aiNodeAnim* FindNodeAnim(const std::string nodeName);
	uint32_t FindScaling(const aiNodeAnim* nodeAnim, float animTime);
	uint32_t FindRotation(const aiNodeAnim* nodeAnim, float animTime);
	uint32_t FindPosition(const aiNodeAnim* nodeAnim, float animTime);
	vec3 CalcInterpolatedScaling(const aiNodeAnim* nodeAnim, float animTime);
	quat CalcInterpolatedRotation(const aiNodeAnim* nodeAnim, float animTime);
	vec3 CalcInterpolatedPosition(const aiNodeAnim* nodeAnim, float animTime);
	mat4 CalcLocalTransform(NodeTree::Item* item);
	void UpdateLocalTransforms();
	void UpdateWorldTransforms();
	void Update(float delta);
	uint32_t GetBonePrimCount() const;
	void RenderBone(std::vector<VertexPC>* primitive);


};

