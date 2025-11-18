#pragma once
#include "framework.h"

#include "Vertex.h"
#include "Texture2D.h"
#include "MyUtil.h"
#include "MyMath.h"

#define USE_ANIMCURVE false	// モーションデータをLERPデータで持つ
using AnimCurveFloat = MyMath::AnimCurve<float>;
using AnimCurveVec3 = MyMath::AnimCurve<glm::vec3>;
using AnimCurveQuat = MyMath::AnimCurve<glm::quat>;

#define USE_BEZIER_INTERP true




namespace mmd {

#pragma pack(push,1)
	struct PMDVertex
	{
		// 38 bytes
		float pos[3];     // vertex
		float normal[3];  // normal
		float uv[2];      // uv
		uint16_t bone[2]; // bone1, bone2
		uint8_t weight;   // [0,100]. bone1: weight, bone2: (100-weight);
		uint8_t edge;     // 0: on, 1: off
	};

	struct PMDMaterial
	{
		// 70 bytes
		float diffuse[3];
		float alpha;
		float specularity;
		float specular[3];
		float ambient[3];
		uint8_t toon_index; // toon??.bmp // 0.bmp:0xFF, 1.bmp:0x00 ... 10.bmp:0x09
		uint8_t edge_flag;  // contour, shadow
		uint32_t vertex_count;
		char texture_filename[20];
	};

	struct PMDBone
	{
		// 39 bytes
		char bone_name[20];
		uint16_t parent_bone_index; // root=0xFFFF
		uint16_t tail_bone_index;   // tail=0xFFFF
		uint8_t bone_type; // 0:rot, 1:rot+trans, 2:IK, 3:?, 4:AffectedbyIK,
		// 5:AfftectedByRot, 6:IKTarget, 7:Invisible
		// ver4.0~ 8:twist, 9:rot
		uint16_t ik_parent_bone_index; // NoIK=0
		float bone_pos[3];
	};

	struct PMDIK
	{
		// 11 + 2 * chain_length bytes
		uint16_t bone_index;
		uint16_t target_bone_index;
		uint8_t chain_length;
		uint16_t iterations;
		float weight;
		// uint16_t  child_bone_indices[]; // len=chain_length
	};

	struct PMDMorphVertex
	{
		// 4+4*3
		uint32_t vertex_index;
		float pos[3];
	};

	struct PMDMorph
	{
		char name[20];
		uint32_t vertex_count;
		uint8_t type;
		// PMDMorphVertex  vertices[];   // len = vertex_count;
	};
#pragma pack(pop)

	struct Motion
	{
		int frameNo;
		glm::vec3 pos;
		glm::quat rotation; // quaternion
		glm::u8vec4 interpX;
		glm::u8vec4 interpY;
		glm::u8vec4 interpZ;
		glm::u8vec4 interpR;
	};

	struct Bone
	{
		Bone() :
			name(),
			ascii_name(),
			parentIndex(0xffffu),
			tailIndex(0xffffu),
			type(0),
			parentIndexIK(0xffffu),
			pos(0.0f),
			rotation(1.0f, 0.0f, 0.0f, 0.0f),
			matrix(1.0f),
			matrixTemp(1.0f),
			motions(),
			motionOffsetPos(0.0f),
			isLeg(false),
			isChain(false),
			isPinnedChain(false),
			updated(false),
			min(0.0f),
			max(0.0f),
			dim(0.0f)
		{}
		~Bone() {};

		std::string name;
		std::string ascii_name;
		uint16_t parentIndex;
		uint16_t tailIndex;
		uint8_t type;
		uint16_t parentIndexIK;

		glm::vec4 pos;      // w = 1
		glm::quat rotation; // quaternion
		glm::mat4 bindPose;

		glm::mat4 matrix;
		glm::mat4 matrixTemp; // temporal

		std::vector<Motion> motions;

		glm::vec3 motionOffsetPos;

		bool isLeg;
		bool isChain;
		bool isPinnedChain;
		bool updated;

		glm::vec3 min;
		glm::vec3 max;
		glm::vec3 dim;
	};

	struct IK
	{
		uint16_t boneIndex;
		uint16_t targetBoneIndex;
		uint8_t chainLength;
		uint16_t iterations;
		float weight;
		std::vector<uint16_t> childBoneIndices;
	};

	struct Morph
	{
		std::string name;
		uint32_t vertexCount;
		uint8_t type;
		std::vector<PMDMorphVertex> vertices;
		std::vector<Motion> motions;
	};
	
	class PMD
	{
	public:
		PMD() {};
		~PMD() {};

		size_t GetVerticesCount() const
		{
			return vertices_.size();
		}
		std::vector<PMDVertex> vertices_;
		std::vector<uint16_t> indices_;
		std::vector<PMDMaterial> materials_;
		std::vector<Texture2D*> textures_;
		std::vector<Bone> bones_;
		std::vector<IK> iks_;
		std::vector<Morph> morphs_;

		std::string name_;
		std::string comment_;
		float version_;
	};


#pragma pack(push,1)
	struct VMDMotion
	{
		// 111 bytes
		char bone_name[15];
		uint32_t frame_no;
		float location[3];
		float rotation[4];               // quaternion
		uint8_t interpolation[64];
		//interpX(interpolation[0], interpolation[4], interpolation[8], interpolation[12])
		//interpY(interpolation[1], interpolation[5], interpolation[9], interpolation[13])
		//interpZ(interpolation[2], interpolation[6], interpolation[10], interpolation[14])
		//interpR(interpolation[3], interpolation[7], interpolation[11], interpolation[15])
	};

	struct VMDMorph
	{
		// 23 bytes
		char morph_name[15];
		uint32_t frame_no;
		float weight;
	};

#pragma pack(pop)

	
	class VMD
	{
	public:
		VMD() {};
		~VMD() {};
		float start_ = 0.0f;
		float end_ = 0.0f;
		std::vector<VMDMotion> motions_;
		std::vector<VMDMorph> morphs_;
#if USE_ANIMCURVE
		std::unordered_map<std::string, AnimCurveVec3> Vec3Curves_;
		std::unordered_map<std::string, AnimCurveQuat> QuatCurves_;
#endif
		std::unordered_map<std::string, AnimCurveFloat> MorphCurves_;

		std::string name_;
	};

	class PMDReader
	{
	public:
		PMDReader() { InitSjisTbl(); }
		~PMDReader() {}

		bool InitSjisTbl();
		PMD* LoadFromStream(std::istream& is);
		PMD* LoadFromFile(const std::string& filename);

	private:
	};



	class VMDReader
	{
	public:

		VMD* LoadFromStream(std::istream& is);
		VMD* LoadFromFile(const std::string& filename);

	private:
	};



	class MMDScene
	{
	public:
		MMDScene();
		~MMDScene();

		bool LoadPMD(const char* pmdmodel, const char* texture_dir);
		bool LoadPMDStream(std::istream& is);
		bool LoadVMD(const char* vmdmodel);
		bool LoadVMDStream(std::istream& is);

		void ClearUpdateFlags(int rootIndex, int boneIndex, mmd::PMD* model);
		void ClearUpdateFlags(std::vector<mmd::Bone>& bones);

		// Update bone position(resolve IK).
		void UpdateBone(float frame, float step);

		void SetModel(PMD* model) { model_ = model; }

		PMD* GetModel() const { return model_; }
		size_t GetVerticesCount() const
		{
			return model_ ? (model_->GetVerticesCount()) : 0;
		}

		void AttachAnimation(VMD* anim);
		VMD* GetAnimation() const { return anim_; }

		float* renderVertices = nullptr;
		mat4 root_ = mat4(1.0f);;
//		std::unordered_map<std::string, Texture2D*> textures_;
		std::vector<vec3> deformBuffer;
		std::vector<vec3> morphBuffer;
		float* Prepare();
		void Update(float delta);
		void UpdateMorph();
		uint32_t GetDeformMatrixCount() const;
		void GetDeformMatrix(mat4* dst, size_t dstCount) const;

		void IKSolve(IK* ik, float errToleranceSq);

		void SetBoneMatrix(int idx, Bone& bone, float frame);
		void UpdateBoneMatrix(Bone& bone);
		void UpdateBoneMatrix(Bone* bone, PMD* model);
		void GetCurrentBoneMatrix(mat4& mat, Bone& bone, PMD* model);
		void GetCurrentBonePosition(vec3& v, Bone& bone, PMD* model);

		float BezierEval(const glm::u8vec4& ip, float t);
		struct MotionSegment
		{
			int m0;
			int m1;
		};
		MotionSegment FindMotionSegment(float frame, std::vector<Motion>& motions);
		void InterpolateMotion(quat& rotation, vec3& position, std::vector<Motion>& motions, float frame);

		float GetFrameTime() const;
		float current_frame = 0.0f;
		void VertexTransform();
		uint32_t GetMaterialCount() const;
		PMDMaterial& GetMaterial(uint32_t materialIdx) const;
		Texture2D* GetTexture(uint32_t materialIdx) const;
		void SetTexture(uint32_t materialIdx, Texture2D* tex);
		void DrawMesh(uint32_t materialIdx, std::vector<VertexPNT>* vert);
		void DrawBone(std::vector<VertexPC>* pc);
		void DrawBoneBbox(std::vector<VertexPC>* pc);
	private:
		void IdentifyChainBones(std::string seed_name, std::set<std::string>* exception_list);
		void CalcBbox();

		PMD* model_;
		VMD* anim_;
	};

}//namespace

