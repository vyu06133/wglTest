#pragma once
#include "framework.h"
//------------------------------------------
#include "Vertex.h"
#include "Texture2D.h"
#include "obb.h"



#define ATTRIBUTE_FLAG_NULL		0x00000002
#define ATTRIBUTE_FLAG_SKELETON	0x00000008
#define ATTRIBUTE_FLAG_MESH		0x00000010
#define ATTRIBUTE_FLAG_CAMERA	0x00000080
#define ATTRIBUTE_FLAG_LIGHT	0x00000400

#define SOFTWARE_DEFORM 0

#define USE_GLM 1
#if USE_GLM
inline static vec3 MyTransform(const glm::mat4& m, const vec3& v)
{
	auto v4 = m * vec4(v.x, v.y, v.z, 1.0f);
	return vec3(v4.x, v4.y, v4.z);
}

inline static glm::mat3 ComputeNormalMatrix(const glm::mat4& m)
{
	// 逆行列の転置行列
	auto normalmatrix = glm::transpose(glm::inverse(glm::mat3(m)));
	return glm::mat3(normalmatrix);
}

inline static vec3 MyTransformNormal(const glm::mat4& m, const vec3& v)
{
	auto v3 = glm::normalize(ComputeNormalMatrix(m) * vec3(v.x, v.y, v.z));
	return vec3(v3.x, v3.y, v3.z);
}

inline static glm::mat4 ToMyMat(const fbxsdk::FbxMatrix& fbxMat)
{
	glm::mat4 glmMat;

	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			glmMat[row][col] = static_cast<float>(fbxMat.Get(row, col));
		}
	}

	return glmMat;
}
inline static glm::mat4 ToMyMat(const fbxsdk::FbxAMatrix fbxAMat)
{
	fbxsdk::FbxMatrix fbxMat(fbxAMat);
	return ToMyMat(fbxMat);
}
#else
using MyMat = DirectX::SimpleMath::Matrix;
inline static SimpleMath::Vector3 MyTransform(const SimpleMath::Matrix& m, const SimpleMath::Vector3& v)
{
	return Vector3::Transform(v, m);
}

inline static SimpleMath::Vector3 MyTransformNormal(const SimpleMath::Matrix& m, const SimpleMath::Vector3& v)
{
	return Vector3::TransformNormal(v, m);
}

inline static Matrix ToMyMat(const fbxsdk::FbxMatrix& fbxMat)
{
	Matrix dxMat;

	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			dxMat(row, col) = static_cast<float>(fbxMat.Get(row, col));
		}
	}

	return dxMat;
}

inline static Matrix ToMyMat(const fbxsdk::FbxAMatrix fbxAMat)
{
	fbxsdk::FbxMatrix fbxMat(fbxAMat);
	return ToMyMat(fbxMat);
}

inline static Matrix ToMyMat(const SimpleMath::Matrix& m)
{
	return m;
}

#endif

using PrimitiveBatchPC = int;
using PrimitiveBatchPNCTAW = int;

class FBXAsset
{
public:
	FBXAsset();
	~FBXAsset();
	std::string ModifyPathW(const wchar_t* path)
	{
		return std::string("");
	}
	std::string ModifyPath(const char* dir, const char* path)
	{
		return std::string(dir) + "\\" + MyUtil::StripPath(path);
	}
	struct MaterialInfo
	{
		MaterialInfo() = default;
		~MaterialInfo()
		{
			if (texture)
			{
				delete texture;
				texture = nullptr;
			}
		}
		vec4 diffuse = vec4(0.0f);
		std::string textureFile = "";
		Texture2D* texture = nullptr;
		vec4 specular = vec4(0.0f);
		float shininess = 0.0f;
		vec4 ambient = vec4(0.0f);
		vec4 emmisive = vec4(0.0f);
		inline std::string ToString() const
		{
			std::stringstream ss;
			ss << "ambient=" << glm::to_string(ambient) << std::endl;
			ss << "diffuse=" << glm::to_string(diffuse) << std::endl;
			ss << "emmisive=" << glm::to_string(emmisive) << std::endl;
			ss << "specular=" << glm::to_string(specular) << std::endl;
			ss << "shininess=" << shininess << std::endl;
			ss << "textureFile=" << textureFile << std::endl;
			return ss.str();
		}
		inline void Command()
		{
			//if (texture)
			//{
			//	glEnable(GL_TEXTURE_2D);
			//	texture->BindTexture();
			//}
			//else
			//{
			//	glDisable(GL_TEXTURE_2D);
			//}
			//glDisable(GL_COLOR_MATERIAL);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(diffuse));
			//if (shininess)
			//{
			//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(specular));
			//	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
			//}
			//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(ambient));
			//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glm::value_ptr(emmisive));
		}
	};

	struct ClusterInfo
	{
		ClusterInfo() = default;
#if USE_CLUSTER_INDICES
		int indicesCount = 0;
		std::vector<uint32_t> indices;
		std::vector<float> weights;
#endif
		fbxsdk::FbxCluster* cluster = nullptr;
		std::string boneName;
		mat4 ClusterTransform;
		mat4 ClusterTransformLink;
		OBB obb;
		inline void SetTransformMatrix(const mat4& m)
		{
			ClusterTransform = m;
		}
		inline void GetTransformMatrix(mat4* pMatrix) const
		{
			*pMatrix = ClusterTransform;
		}
		inline void SetTransformLinkMatrix(const mat4& m)
		{
			ClusterTransformLink = m;
		}
		inline void GetTransformLinkMatrix(mat4* pMatrix) const
		{
			*pMatrix = ClusterTransformLink;
		}
	};
	struct SkinInfo
	{
		SkinInfo() = default;
		fbxsdk::FbxSkin* skin =nullptr;
		std::vector<ClusterInfo> clusters;
	};
	struct MeshInfo
	{
		MeshInfo() = default;
		~MeshInfo()
		{
			for (auto& m : materials_)
			{
				if (m)
				{
					delete m;
					m = nullptr;
				}
			}
		}
		std::string name;
		fbxsdk::FbxNode* node = nullptr;
		fbxsdk::FbxMesh* mesh = nullptr;
		SkinInfo skin;
		mat4 GeometryOffset;
		uint32_t pointsCount = 0;
		uint32_t faceCount = 0;
		std::vector<VertexPNCW> cp_;
		std::vector<VertexPNCTAW> vbuf_;
		std::vector<VertexPNCT> deform_;
		std::vector<uint16_t> ibuf_;
		std::vector<uint32_t> MaterialIndex_;
		std::vector<MaterialInfo*> materials_;
	};
	std::vector<MeshInfo*> m_meshes;

	struct NodeInfo
	{
		NodeInfo(fbxsdk::FbxNode* src = nullptr) :
			name(src ? src->GetName() : ""),
			type(),
			flags(0),
			id(0),
			node(src),
			scalingOffset(1.0f),
			scalingPivot(0.0f),
			rotationOffset(0.0f),
			rotationPivot(0.0f),
			preRotation(),
			postRotation(),
			lclScaling(""),
			lclRotation(""),
			lclTranslation(""),
			worldTransform(1.0f),
			localTransform(1.0f)
		{}
		std::string name;
		std::string type;
		uint32_t flags;
		uint32_t id;
		fbxsdk::FbxNode* node;
		vec3 scalingOffset;
		vec3 scalingPivot;
		vec3 rotationOffset;
		vec3 rotationPivot;
		quat preRotation;
		quat postRotation;
		std::string lclScaling;
		std::string lclRotation;
		std::string lclTranslation;
		mat4 worldTransform;
		mat4 localTransform;
	};

	using NodeTree = MyUtil::Tree<NodeInfo>;
	NodeTree m_nodeTree;
	using NodeMap = std::unordered_map < std::string, NodeTree::Item*>;
	NodeMap	m_nodeMap;

	inline static std::string MakeAnimName(fbxsdk::FbxNode* node, fbxsdk::FbxProperty* prop)
	{
		std::string name;// 1文字目は{'S'|'R'|'T'}
		std::string nodeName = node->GetName();
		std::string propName = prop->GetName().Buffer();
		if (propName == "Lcl Scaling")
			name = "Scl";
		else if (propName == "Lcl Rotation")
			name = "Rot";
		else if (propName == "Lcl Translation")
			name = "Tra";
		else
			name = "???";
		name += "@";
		name += nodeName;
		return name;
	}
	inline static char ResolveAnimType(const std::string& name)
	{
		return name[0];
	}
	using AnimCurveVec3 = MyMath::AnimCurve<vec3>;
	using AnimCurveQuat = MyMath::AnimCurve<quat>;
	AnimCurveVec3* ParseProperty(fbxsdk::FbxNode* node, fbxsdk::FbxPropertyT<fbxsdk::FbxDouble3>& property);
	struct DopeSheet
	{
		~DopeSheet()
		{
			for (auto& itr : curves)
			{
				if (itr.second)
				{
					delete itr.second;
					itr.second = nullptr;
				}
			}
			curves.clear();
		}
		std::string name;
		fbxsdk::FbxAnimStack* stack_ = nullptr;
		float start_ = 0.0f;
		float stop_ = 0.0f;
		std::unordered_map<std::string, MyMath::AnimCurveBase*> curves;// TODO:型安全性を考慮すべき
	};
	struct SamplePoint
	{
		~SamplePoint()
		{
			answers.clear();
		}
		union Answer
		{
			Answer() : q(1.0f, 0.0f, 0.0f, 0.0f){}
			vec3 xyz;
			quat q;
		};
		std::unordered_map<std::string, Answer> answers;// TODO:複数SamplePoint間で補間出来るようにする
		float sampleTime_ = 0.0f;
		SamplePoint* Duplicate()
		{
			auto newsp = _NEW SamplePoint();
			ASSERT(newsp);
			if (!newsp)
			{
				return nullptr;
			}
			newsp->sampleTime_ = sampleTime_;
			for (auto& itr : answers)
			{
				newsp->answers[itr.first] = itr.second;
			}
			return newsp;
		}
		inline void Interp(SamplePoint& src, float alpha)
		{
			for (auto& itr : src.answers)
			{
				switch (ResolveAnimType(itr.first))
				{
				case 'R':
					answers[itr.first].q = glm::slerp(answers[itr.first].q, src.answers[itr.first].q, alpha);
					break;
				case 'S':
				case 'T':
				default:
					answers[itr.first].xyz = glm::mix(answers[itr.first].xyz, src.answers[itr.first].xyz, alpha);
					break;
				}
			}
		}
	};
	DopeSheet* m_currentDopeSheet = nullptr;
	SamplePoint m_samplePoint;
	SamplePoint m_prevSamplePoint;
	float m_sampleAlpha = 0.0f;
	std::unordered_map<std::string, DopeSheet*> m_dopeSheets;
	std::vector<std::string> m_animStackNames;
	bool SelectAnim(int number);
	bool LoadAsset(const std::string& Filename);
	bool AddAnim(const std::string& Filename);	//TODO:ノード/プロパティの変換テーブルを設ける
	bool ParseMaterial(MeshInfo* mi);
	bool ParseMesh(NodeTree::Item* item);
	bool ParseMeshCP(MeshInfo* meshinfo);
	int32_t AddOrFindVbuf(MeshInfo* meshinfo, const VertexPNCTAW& v);
	bool ParseMeshAttr(MeshInfo* meshinfo);
	void DeleteAsset();
	void Update(float delta);
	void Render();
	uint32_t GetMeshCount() const;
	uint32_t GetBoneCount(uint32_t mesh) const;
	uint32_t GetDeformation(uint32_t mesh, mat4* dst, size_t dstsize) const;
	
	uint32_t GetMaterialCount(uint32_t mesh) const;
	MaterialInfo* GetMaterial(uint32_t mesh, uint32_t material) const;
	void GetPrimitive(uint32_t mesh, uint32_t material, PrimitiveBatchPNCTAW* primitive);

	uint32_t GetWirePrimCount()const;
	uint32_t GetBonePrimCount()const;
	uint32_t GetMeshPrimCount()const;
	void RenderMesh(std::vector<VertexPNCTAW>* primitive);
	void RenderWire(std::vector<VertexPNCT>* primitive);
	void RenderBone(std::vector<VertexPC>* primitive);
	void GetWirePrim(uint32_t mesh, uint32_t material, std::vector<VertexPNCT>* primitive);
	void GetMeshPrim(uint32_t mesh, uint32_t material, std::vector<VertexPNCTAW>* primitive);

	bool ParsePose(NodeTree* tree, fbxsdk::FbxScene* scene, const std::string& prefix);
	bool ParseAnimStack(NodeTree* tree, fbxsdk::FbxScene* scene, const std::string& prefix);
	void BuildNodeTree(NodeTree* tree, uint32_t id, fbxsdk::FbxNode* obj, NodeTree::Item* parent);
	void BuildNodeMap(NodeMap* NodeMap, NodeTree* tree);
	bool IsVaild() const { return m_scene != nullptr; }
public:
	void DeformBoneWeight();
	mat4 ComputeClusterDeformation(MeshInfo* mesh,const ClusterInfo& ci) const;

	std::string m_currentAssetPath;
	fbxsdk::FbxManager* m_manager = nullptr;
	fbxsdk::FbxScene* m_scene = nullptr;
	std::vector<fbxsdk::FbxPose*> m_poses;
	float m_time = 0.0f;
	float m_secPerFrame = 1.0f / 60.0f;
	float m_start = 0.0f;
	float m_stop = 0.0f;
	mat4 m_rootTransform;
	void SetRootTransform(const mat4& m);
	bool mode_ = false;
	void modechg()
	{
		mode_ = !mode_;
		TRACE("mode_=%d\n", mode_);
	}
	void UpdateLocalTransforms();
	void UpdateWorldTransforms();
	void UpdateSamplePoint(float time);
	mat4 CalcLocalTransform(NodeTree::Item* item);
	mat4 CalcWorldTransform(NodeTree::Item* item);
};
