#pragma once
#include "framework.h"
//------------------------------------------
#include "Vertex.h"
#include "Texture2D.h"
#include "obb.h"

#if 1
// open fbx
#include "ufbx.h"
#else
// fbxsdk
#pragma warning(push)
#pragma warning(disable:26812)	// enum Class
#pragma warning(disable:26451)	// 演算オーバーフロー
#pragma warning(disable:26495)	// メンバ初期化
#include <fbxsdk.h>
#include <fbxsdk/core/math/fbxmath.h>
#include <fbxsdk/core/math/fbxdualquaternion.h>
#include <fbxsdk/core/math/fbxmatrix.h>
#include <fbxsdk/core/math/fbxquaternion.h>
#pragma warning(pop)
using fbxsdk::FbxAMatrix;
using fbxsdk::FbxMatrix;
#endif

#define USE_DOPESHEET   1

#define ATTRIBUTE_FLAG_NULL		0x00000002
#define ATTRIBUTE_FLAG_SKELETON	0x00000008
#define ATTRIBUTE_FLAG_MESH		0x00000010
#define ATTRIBUTE_FLAG_CAMERA	0x00000080
#define ATTRIBUTE_FLAG_LIGHT	0x00000400



using PrimitiveBatchPC = int;
using PrimitiveBatchPNCTAW = int;

class UFBXAsset
{
public:
#if 0
	template<typename T>
	struct AnimCurve
	{
		std::string name;
		T currentValue;
		T defaultValue;
		typedef std::pair<float, T> KeyFrame;
		std::vector<KeyFrame> keys;
		AnimCurve() = default;
		void Init()
		{
			keys.clear();
		};
		~AnimCurve()
		{
			keys.clear();
		};
		T GetDefaultValue()
		{
			return defaultValue;
		}
		T SetDefaultValue(const T& v)
		{
			defaultValue = v;
			return defaultValue;
		}
		T GetCurrentValue()
		{
			return currentValue;
		}
		T SetCurrentValue(const T& v)
		{
			currentValue = v;
			return currentValue;
		}
		void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					//時刻が同じなので値上書き
					keys[i].second = value;
					return;
				}
			}
			//追加
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time)
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return SetCurrentValue(defaultValue);
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					return SetCurrentValue(MyMath::Interp(key1.second, key2.second, t));
				}
			}
			return SetCurrentValue(defaultValue);
		}
	};
	bool Vec3CurveToQuatCurve(AnimCurve<quat>* dst, AnimCurve<vec3>* src)
	{
		const float RAD = MyMath::_PAI / 180.0f;
		dst->Init();
		dst->name = src->name;
		dst->SetDefaultValue(quat(src->defaultValue * RAD));
		for (const auto& itr : src->keys)
		{
			auto& v = itr.second;
			dst->Insert(itr.first, quat(v * RAD));
		}
		return true;
	}
#endif
	UFBXAsset();
	~UFBXAsset();
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
		const ufbx_material* ufbxMaterial = nullptr;
		vec3 diffuse = vec3(0.0f);
		std::string textureFile = "";
		Texture2D* texture = nullptr;
		vec3 specular = vec3(0.0f);
		float shininess = 0.0f;
		vec3 ambient = vec3(0.0f);
		vec3 emmisive = vec3(0.0f);
		bool valid = false;
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
	std::vector<MaterialInfo*> m_materials;

	struct ClusterInfo
	{
		ClusterInfo() = default;
#if USE_CLUSTER_INDICES
		int indicesCount = 0;
		std::vector<uint32_t> indices;
		std::vector<float> weights;
#endif
		const ufbx_skin_cluster* cluster = nullptr;
		std::string boneName;
		mat4 ClusterTransformLink;
		//OBB obb;
		std::string ToString() const
		{
			std::stringstream ss;
			ss << "■ClusterInfo" << std::endl;
			ss << "boneName " << boneName << "" << std::endl;
			ss << "TransformLink " << glm::to_string(ClusterTransformLink) << std::endl;
			return ss.str();
		}
		inline void SetTransformLinkMatrix(const mat4& m)
		{
			ClusterTransformLink = m;
		}
		inline void GetTransformLinkMatrix(mat4* pMatrix) const
		{
			*pMatrix = ClusterTransformLink;
		}
		inline mat4 GetTransformLinkMatrix() const
		{
			return ClusterTransformLink;
		}
	};
	struct SkinInfo
	{
		SkinInfo() = default;
		const ufbx_skin_deformer* skin = nullptr;
		std::vector<ClusterInfo> clusters;
	};
	struct Geometry
	{
		Geometry() = default;
		Geometry(int32_t mat) :material(mat) {};
		int32_t material = 0;
		uint32_t pointsCount = 0;
		std::vector<VertexPNCTAW> vbuf;
		std::vector<VertexPNCTA> deform;
		std::vector<uint16_t> ibuf;
	};
	struct MeshInfo
	{
		MeshInfo() = default;
		std::string name;
		const ufbx_node* node = nullptr;
		const ufbx_mesh* mesh = nullptr;
		SkinInfo skin;
		mat4 GeometricMatrix;
		std::vector<Geometry> MaterialGroup;
		uint32_t pointsCount = 0;
		uint32_t faceCount = 0;
		std::vector<VertexPNCTAW> vbuf_;
		std::vector<VertexPNCTA> deform_;
		std::vector<uint16_t> ibuf_;
	};
	std::vector<MeshInfo*> m_meshes;
	using AnimCurveFloat = MyMath::AnimCurve<float>;
	using AnimCurveVec3 = MyMath::AnimCurve<vec3>;
	using AnimCurveQuat = MyMath::AnimCurve<quat>;
	struct NodeInfo
	{
		NodeInfo() :
			name(""),
			type(""),
			flags(0),
			id(0),
			node(nullptr),
			scalingOffset(0.0f),
			scalingPivot(0.0f),
			rotationOffset(0.0f),
			rotationPivot(0.0f),
			preRotation(),
			postRotation(),
#if USE_DOPESHEET
			lclSclCurveLabel(),
			lclRotCurveLabel(),
			lclTraCurveLabel(),
#else
			lclSclCurve(),
			lclRotCurve(),
			lclTraCurve(),
#endif
			worldTransform(1.0f),
			localTransform(1.0f)
		{
		}
		NodeInfo(const ufbx_node* src) :
			name(src->name.data),
			type(""),
			flags(0),
			id(0),
			node(src),
			scalingOffset(0.0f),
			scalingPivot(0.0f),
			rotationOffset(0.0f),
			rotationPivot(0.0f),
			preRotation(),
			postRotation(),
#if USE_DOPESHEET
			lclSclCurveLabel(),
			lclRotCurveLabel(),
			lclTraCurveLabel(),
#else
			lclSclCurve(),
			lclRotCurve(),
			lclTraCurve(),
#endif
			worldTransform(1.0f),
			localTransform(1.0f)
		{
		}
		~NodeInfo()
		{
			node = nullptr;
		}
		std::string name;
		std::string type; // for debug/display
		uint32_t flags;
		uint32_t id;
		const ufbx_node* node;
		vec3 scalingOffset;
		vec3 scalingPivot;
		vec3 rotationOffset;
		vec3 rotationPivot;
		quat preRotation;
		quat postRotation;
#if USE_DOPESHEET
		std::string lclSclCurveLabel;
		std::string lclRotCurveLabel;
		std::string lclTraCurveLabel;
#else
		AnimCurveVec3 lclSclCurve;
		AnimCurveQuat lclRotCurve;
		AnimCurveVec3 lclTraCurve;
#endif
		mat4 worldTransform;
		mat4 localTransform;
	};

	struct DopeSheet
	{
	public:
		DopeSheet() = default;
		std::string name;
		float m_FPS = 0.0f;
		float m_start = 0.0f;
		float m_stop = 0.0f;
		std::unordered_map<std::string, std::variant<AnimCurveVec3, AnimCurveQuat>> curveMap;
		
		inline static std::string MakeLabel(const ufbx_node* node, const char* propName)
		{
			std::string name;
			std::string nodeName = node ? node->name.data : "";
			if (strcmp(propName, "Lcl Scaling") == 0)
				name = "Scl";
			else if (strcmp(propName, "Lcl Rotation") == 0)
				name = "Rot";
			else if (strcmp(propName, "Lcl Translation") == 0)
				name = "Tra";
			else
				name = "???";
			name += "@";
			name += nodeName;
			return name;
		}
	};
	std::vector<DopeSheet> m_dopeSheets;
	DopeSheet* m_CurSheet;
	DopeSheet m_sheet;

	using NodeTree = MyUtil::Tree<NodeInfo>;
	NodeTree m_nodeTree;
	using NodeMap = std::unordered_map < std::string, NodeTree::Item*>;
	NodeMap	m_nodeMap;

	inline static char ResolveAnimType(const std::string& name)
	{
		return name[0];
	}

	bool ParseAnimCurve(AnimCurveVec3& dst, const ufbx_anim_prop* animProp, const ufbx_vec3& defValue);
	std::vector<std::string> m_animStackNames;
	bool LoadAsset(const std::string& Filename, int anim = 0);
	bool LoadBinary(const std::vector<uint8_t>& data, int anim = 0);
	void BuildNodeTree(NodeTree* tree, uint32_t NodeID, const ufbx_node* node, NodeTree::Item* parent);
	AnimCurveVec3* ParseProperty(const ufbx_node* node, const char* propName, const vec3& defaultValue);
	const ufbx_anim_stack* m_CurAnimStack = nullptr;
	const ufbx_anim_layer* m_CurAnimLayer = nullptr;
	int ParseMaterial(ufbx_material* material);
	bool ParseMesh(NodeTree::Item* item);
	bool ParseMesh_BoneWeight(MeshInfo* meshinfo);
	bool ParseMaterialGroup(MeshInfo* meshinfo);
	void GetDeformed(std::vector<VertexPNCTAW>* pnctaw, uint32_t meshIndex = 0, uint32_t material = 0);
	void DeleteAsset();
	void Update(float delta);
	int GetDeformMatrixCount(uint32_t mesh) const;
	void GetDeformMatrix(uint32_t mesh, mat4* dst, size_t dstCount) const;
	void GetDeformMatrix(const MeshInfo* mesh, mat4* dst, size_t dstCount) const;
	void GetSkeleton(std::vector<VertexPC>* primitive);
	void GetVBuf(uint32_t mesh, uint32_t material, std::vector<VertexPNCTAW>* vbuf) const;
	void Render();
	uint32_t GetMeshCount() const;
	uint32_t GetBoneCount(uint32_t mesh) const;
	MeshInfo* GetMeshInfo(uint32_t mesh) const;

	uint32_t GetMaterialGroupCount(uint32_t mesh) const;
	MaterialInfo* GetMaterialInfo(uint32_t info) const;
	Geometry* GetMaterialGeometry(uint32_t mesh, uint32_t group) const;

	uint32_t GetBonePrimCount()const;
	uint32_t GetMeshPrimCount()const;
	void RenderMesh(std::vector<VertexPNCTAW>* primitive);
	void RenderWire(std::vector<VertexPNCT>* primitive);
#if 0
	void GetWirePrim(uint32_t mesh, uint32_t material, std::vector<VertexPNCT>* primitive);
	void GetMeshPrim(uint32_t mesh, uint32_t material, std::vector<VertexPNCTAW>* primitive);
#endif
	bool IsVaild() const { return m_scene != nullptr; }
public:
	void DeformBoneWeight();
	mat4 ComputeClusterDeformation(const MeshInfo* mesh, const ClusterInfo& ci) const;

	std::string m_currentAssetPath;
	ufbx_scene* m_scene = nullptr;
	float m_time = 0.0f;
	mat4 m_rootTransform;
	mat4 m_adjust;
	void EnsureYUp(const vec3& scale = vec3(1.0f));
	void SetRootTransform(const mat4& m);
	bool mode_ = false;
	void modechg()
	{
		mode_ = !mode_;
		TRACE("mode_=%d\n", mode_);
	}
	void UpdateLocalTransforms();
	void UpdateWorldTransforms();
	mat4 CalcLocalTransform(NodeTree::Item* item);
	mat4 CalcWorldTransform(NodeTree::Item* item);
};
