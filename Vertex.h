//
// Created by vyu06133 on 2025/05/29 木.
//

#pragma once
#include "framework.h"

#pragma pack(push,1)
struct Vertex
{
	template <typename T, typename M>
	constexpr inline static size_t offset_of(M T::* member)
	{
		return reinterpret_cast<size_t>(&(reinterpret_cast<T*>(0)->*member));
	}

public:
	inline static void DisableAttribAll(GLuint program)
	{
		GLint maxAttribs = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
		for (int i = 0; i < maxAttribs; i++)
		{
			glDisableVertexAttribArray(i);
		}
	}

	inline static void EnableAttrib(GLuint program, const char* name, GLint size, GLenum type,
		GLsizei stride, size_t ofs, const void* baseptr = nullptr)
	{
		//TRACE("%s %dx%s %d %zu\n", name, size, glEnumMap[type], stride, ofs);
		GLint loc = glGetAttribLocation(program, name);
		if (loc >= 0)
		{
			if (type == GL_BYTE || type == GL_UNSIGNED_BYTE ||
				type == GL_SHORT || type == GL_UNSIGNED_SHORT ||
				type == GL_INT || type == GL_UNSIGNED_INT)
			{
				glVertexAttribIPointer(loc, size, type, stride, (void*)((char*)baseptr + ofs));
				glEnableVertexAttribArray(loc);
			}
			else if (type == GL_FLOAT)
			{
				glVertexAttribPointer(loc, size, type, GL_FALSE, stride, (void*)((char*)baseptr + ofs));
				glEnableVertexAttribArray(loc);
			}
			else
			{
				assert(false);
			}
		}
	}
	template<typename T>
	inline static bool BindAttributes(GLuint program, const T* baseptr = nullptr)
	{
		T::BindAttributes(program, baseptr);
		return true;
	}
};

struct VertexP : public Vertex
{
public:
	vec3 pos;

	inline void Reset()
	{
		*this = VertexP();
	}
	VertexP() : pos(0.0f) {}
	VertexP(const vec3& P) : pos(P) {}
	VertexP(const float& X, const float& Y, const float& Z) : pos(X, Y, Z) {}

	inline static void BindAttributes(GLuint program, const VertexP* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexP), offset_of(&VertexP::pos), baseptr);
	}
};

struct VertexPNC : public Vertex
{
public:
	vec3 pos;
	vec3 normal;
	vec4 color;
	inline void Reset()
	{
		*this = VertexPNC();
	}
	VertexPNC() : pos(0.0f), normal(0.0f), color(1.0f) {}
	VertexPNC(const vec3& P) : pos(P), normal(0.0f), color(1.0f) {}
	VertexPNC(const vec3& P, const vec3& N, const vec4& C) : pos(P), normal(N), color(C) {}
	VertexPNC(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& R, const float& G, const float& B, const float& A)
		: pos(PX, PY, PZ), normal(NX, NY, NZ), color(R, G, B, A) {
	}

	inline static void BindAttributes(GLuint program, const VertexPNC* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNC), offset_of(&VertexPNC::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNC), offset_of(&VertexPNC::normal), baseptr);
		Vertex::EnableAttrib(program, "a_color", 4, GL_FLOAT, sizeof(VertexPNC), offset_of(&VertexPNC::color), baseptr);
	}
};

struct VertexPC : public Vertex
{
public:
	vec3 pos;
	vec4 color;
	inline void Reset()
	{
		*this = VertexPC();
	}
	VertexPC() : pos(), color(1.0f) {}
	VertexPC(const vec3& P) : pos(P), color(1.0f) {}
	VertexPC(const vec3& P, const vec4& C) : pos(P), color(C) {}
	VertexPC(const float& PX, const float& PY, const float& PZ,
		const float& R, const float& G, const float& B, const float& A)
		: pos(PX, PY, PZ), color(R, G, B, A) {
	}

	inline static void BindAttributes(GLuint program, const VertexPC* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPC), offset_of(&VertexPC::pos), baseptr);
		Vertex::EnableAttrib(program, "a_color", 4, GL_FLOAT, sizeof(VertexPC), offset_of(&VertexPC::color), baseptr);
	}
};

struct VertexPCT : public Vertex
{
public:
	vec3 pos;
	vec3 normal;
	vec4 color;
	vec2 texcoord;
	inline void Reset()
	{
		*this = VertexPCT();
	}
	VertexPCT() : pos(1.0f), color(1.0f), texcoord(0.0f) {}
	VertexPCT(const VertexPNC& PNC) : pos(PNC.pos), color(PNC.color), texcoord(0.0f) {}
	VertexPCT(const vec3& P, const vec4& C, const vec2& T) : pos(P), color(C), texcoord(T) {}
	VertexPCT(const float& PX, const float& PY, const float& PZ,
		const float& R, const float& G, const float& B, const float& A,
		const float& U, const float& V)
		: pos(PX, PY, PZ), color(R, G, B, A), texcoord(U, V) {
	}

	inline static void BindAttributes(GLuint program, const VertexPCT* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPCT), offset_of(&VertexPCT::pos), baseptr);
		Vertex::EnableAttrib(program, "a_color", 4, GL_FLOAT, sizeof(VertexPCT), offset_of(&VertexPCT::color), baseptr);
		Vertex::EnableAttrib(program, "a_texcoord", 2, GL_FLOAT, sizeof(VertexPCT), offset_of(&VertexPCT::texcoord), baseptr);
	}
};

struct VertexPNCT : public Vertex
{
public:
	vec3 pos;
	vec3 normal;
	vec4 color;
	vec2 texcoord;
	inline void Reset()
	{
		*this = VertexPNCT();
	}
	VertexPNCT() : pos(1.0f), normal(1.0f), color(1.0f), texcoord(0.0f) {}
	VertexPNCT(const VertexPNC& PNC) : pos(PNC.pos), normal(PNC.normal), color(PNC.color), texcoord(0.0f) {}
	VertexPNCT(const vec3& P, const vec3& N, const vec4& C, const vec2& T) : pos(P), normal(N), color(C), texcoord(T) {}
	VertexPNCT(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& R, const float& G, const float& B, const float& A,
		const float& U, const float& V)
		: pos(PX, PY, PZ), normal(NX, NY, NZ), color(R, G, B, A), texcoord(U, V) {
	}

	inline static void BindAttributes(GLuint program, const VertexPNCT* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNCT), offset_of(&VertexPNCT::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNCT), offset_of(&VertexPNCT::normal), baseptr);
		Vertex::EnableAttrib(program, "a_color", 4, GL_FLOAT, sizeof(VertexPNCT), offset_of(&VertexPNCT::color), baseptr);
		Vertex::EnableAttrib(program, "a_texcoord", 2, GL_FLOAT, sizeof(VertexPNCT), offset_of(&VertexPNCT::texcoord), baseptr);
	}
};

struct VertexPNT : public Vertex
{
public:
	vec3 pos;
	vec3 normal;
	vec2 texcoord;
	inline void Reset()
	{
		*this = VertexPNT();
	}
	VertexPNT() : pos(1.0f), normal(1.0f), texcoord(0.0f) {}
	VertexPNT(const vec3& P, const vec3& N, const vec2& T) : pos(P), normal(N), texcoord(T) {}
	VertexPNT(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& U, const float& V)
		: pos(PX, PY, PZ), normal(NX, NY, NZ), texcoord(U, V) {
	}

	inline static void BindAttributes(GLuint program, const VertexPNT* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNT), offset_of(&VertexPNT::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNT), offset_of(&VertexPNT::normal), baseptr);
		Vertex::EnableAttrib(program, "a_texcoord", 2, GL_FLOAT, sizeof(VertexPNT), offset_of(&VertexPNT::texcoord), baseptr);
	}
};

struct VertexPNCTA : public Vertex
{
public:
	vec3 pos;
	vec3 normal;
	vec4 color;
	vec2 texcoord;
	vec3 tangent;
	inline void Reset()
	{
		*this = VertexPNCTA();
	}
	VertexPNCTA() : pos(1.0f), normal(1.0f), color(1.0f), texcoord(0.0f), tangent(0.0f) {}
	VertexPNCTA(const vec3& P) : pos(P), normal(0.0f), color(1.0f), texcoord(0.0f), tangent(0.0f) {}
	VertexPNCTA(const VertexPC& PC) : pos(PC.pos), normal(0.0f), color(PC.color), texcoord(0.0f), tangent(0.0f) {}
	VertexPNCTA(const VertexPNC& PNC) : pos(PNC.pos), normal(PNC.normal), color(PNC.color), texcoord(0.0f), tangent(0.0f) {}
	VertexPNCTA(const VertexPNCT& PNCT) : pos(PNCT.pos), normal(PNCT.normal), color(PNCT.color), texcoord(PNCT.texcoord), tangent(0.0f) {}
	VertexPNCTA(const vec3& P, const vec3& N, const vec4& C, const vec2& T, const vec3& A) : pos(P), normal(N), color(C), texcoord(T), tangent(A) {}
	VertexPNCTA(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& R, const float& G, const float& B, const float& A,
		const float& U, const float& V,
		const float& AX, const float& AY, const float& AZ)
		: pos(PX, PY, PZ), normal(NX, NY, NZ), color(R, G, B, A), texcoord(U, V), tangent(AX, AY, AZ) {
	}

	inline static void BindAttributes(GLuint program, const VertexPNCTA* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNCTA), offset_of(&VertexPNCTA::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNCTA), offset_of(&VertexPNCTA::normal), baseptr);
		Vertex::EnableAttrib(program, "a_color", 4, GL_FLOAT, sizeof(VertexPNCTA), offset_of(&VertexPNCTA::color), baseptr);
		Vertex::EnableAttrib(program, "a_texcoord", 2, GL_FLOAT, sizeof(VertexPNCTA), offset_of(&VertexPNCTA::texcoord), baseptr);
		Vertex::EnableAttrib(program, "a_tangent", 3, GL_FLOAT, sizeof(VertexPNCTA), offset_of(&VertexPNCTA::tangent), baseptr);
	}
};

#if 1//Vertex****Wに生のPODタイプとして入れた∵PODでないとうまくシェーダに渡らないことがある
#if 0
#define DeclBoneWeight uint32_t bone[4];float weight[4];
#define ClearBoneWeight() do{std::memset(bone, 0, sizeof(bone));std::memset(weight, 0, sizeof(weight));}while(0)
#define CopyBoneWeight(BONE, WEIGHT)	do{	std::memcpy(bone, BONE, sizeof(BONE));	std::memcpy(weight, WEIGHT, sizeof(WEIGHT));}while(0)
#else
#endif
#define DeclBoneWeight glm::uvec4 bone;glm::vec4 weight;
#define ClearBoneWeight() do{bone=glm::uvec4(0);weight=glm::vec4(0.0f);}while(0)
#define CopyBoneWeight(BONE, WEIGHT)	do{	bone=BONE;	weight=WEIGHT;}while(0)
#else
// ボーンウェイト情報
struct BoneWeight
{
	static const int NUM_BONES_PER_VERTEX = 4;
	//	uint32_t IDs[4]{};
	glm::uvec4 IDs;
	//	float Weights[4]{};
	glm::vec4 Weights;

	BoneWeight() = default;

	bool AddBoneData(uint32_t BoneID, float Weight)
	{
		for (auto i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (Weights[i] == 0.0f)
			{
				IDs[i] = BoneID;
				Weights[i] = Weight;
				return true;
			}
		}
		return false;
	}

	void Sort()
	{
		for (auto i = 0; i < NUM_BONES_PER_VERTEX - 1; i++)
		{
			for (auto j = i + 1; j < NUM_BONES_PER_VERTEX; j++)
			{
				if (Weights[i] < Weights[j])
				{
					std::swap(Weights[i], Weights[j]);
					std::swap(IDs[i], IDs[j]);
				}
			}
		}
	}
};
#endif

struct VertexPNW : public Vertex
{
public:
	static const int NUM_BONES_PER_VERTEX = 4;
	vec3 pos;
	vec3 normal;
	uint32_t bone[NUM_BONES_PER_VERTEX];
	float weight[NUM_BONES_PER_VERTEX];
	inline void Reset()
	{
		*this = VertexPNW();
	}
	inline void ResetBoneWeight()
	{
		ZeroMemory( bone, sizeof(bone) );
		ZeroMemory( weight, sizeof(weight) );
	}
	inline bool AddBoneData(uint32_t BoneID, float Weight)
	{
		//static_assert(_countof(bone) == _countof(weight));
		for (auto i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (weight[i] == 0.0f)
			{
				bone[i] = BoneID;
				weight[i] = Weight;
				return true;
			}
		}
		return false;
	}
	inline void SortBoneWeight()
	{
		//static_assert(_countof(bone) == _countof(weight));
		for (auto i = 0; i < NUM_BONES_PER_VERTEX - 1; i++)
		{
			for (auto j = i + 1; j < NUM_BONES_PER_VERTEX; j++)
			{
				if (weight[i] < weight[j])
				{
					std::swap(weight[i], weight[j]);
					std::swap(bone[i], bone[j]);
				}
			}
		}
	}
	VertexPNW(const VertexPNW& src) : pos(src.pos), normal(src.normal)
	{
		std::memcpy(bone, src.bone, sizeof(bone));
		std::memcpy(weight, src.weight, sizeof(weight));
	}
	VertexPNW() : pos(1.0f), normal(1.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNW(const vec3& P) : pos(P), normal(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNW(const vec3& P, const vec3& N) : pos(P), normal(N)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNW(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& U, const float& V)
		: pos(PX, PY, PZ), normal(NX, NY, NZ)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNW(const vec3& P, const vec3& N, const uint32_t BONE[4], const float WEIGHT[4]) : pos(P), normal(N)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNW(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& U, const float& V,
		const uint32_t BONE[4], const float WEIGHT[4])
		: pos(PX, PY, PZ), normal(NX, NY, NZ)
	{
		std::memcpy(bone, BONE, sizeof(bone));
		std::memcpy(weight, WEIGHT, sizeof(weight));
	}

	inline static void BindAttributes(GLuint program, const VertexPNW* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNW), offset_of(&VertexPNW::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNW), offset_of(&VertexPNW::normal), baseptr);

		// Bone IDs (整数属性: uvec4)
		Vertex::EnableAttrib(program, "a_bone", 4, GL_UNSIGNED_INT, sizeof(VertexPNW), offset_of(&VertexPNW::bone), baseptr);

		// Bone Weights (float属性: vec4)
		Vertex::EnableAttrib(program, "a_weight", 4, GL_FLOAT, sizeof(VertexPNW), offset_of(&VertexPNW::weight), baseptr);
	}
};

struct VertexPNTW : public Vertex
{
public:
	static const int NUM_BONES_PER_VERTEX = 4;
	vec3 pos;
	vec3 normal;
	vec2 texcoord;
	uint32_t bone[NUM_BONES_PER_VERTEX];
	float weight[NUM_BONES_PER_VERTEX];
	inline void Reset()
	{
		*this = VertexPNTW();
	}
	inline void ResetBoneWeight()
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	inline bool AddBoneData(uint32_t BoneID, float Weight)
	{
//		static_assert(_countof(bone) == _countof(weight));
		for (auto i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (weight[i] == 0.0f)
			{
				bone[i] = BoneID;
				weight[i] = Weight;
				return true;
			}
		}
		return false;
	}
	inline void SortBoneWeight()
	{
//		static_assert(_countof(bone) == _countof(weight));
		for (auto i = 0; i < NUM_BONES_PER_VERTEX - 1; i++)
		{
			for (auto j = i + 1; j < NUM_BONES_PER_VERTEX; j++)
			{
				if (weight[i] < weight[j])
				{
					std::swap(weight[i], weight[j]);
					std::swap(bone[i], bone[j]);
				}
			}
		}
	}
	VertexPNTW(const VertexPNTW& src) : pos(src.pos), normal(src.normal), texcoord(src.texcoord)
	{
		std::memcpy(bone, src.bone, sizeof(bone));
		std::memcpy(weight, src.weight, sizeof(weight));
	}
	VertexPNTW() : pos(1.0f), normal(1.0f), texcoord(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNTW(const vec3& P) : pos(P), normal(0.0f), texcoord(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNTW(const vec3& P, const vec3& N, const vec2& T) : pos(P), normal(N), texcoord(T)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNTW(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& U, const float& V)
		: pos(PX, PY, PZ), normal(NX, NY, NZ), texcoord(U, V)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNTW(const vec3& P, const vec3& N, const vec2& T, const uint32_t BONE[4], const float WEIGHT[4]) : pos(P), normal(N), texcoord(T)
	{
		std::memcpy(bone, BONE, sizeof(bone));
		std::memcpy(weight, WEIGHT, sizeof(weight));
	}
	VertexPNTW(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& U, const float& V,
		const uint32_t BONE[4], const float WEIGHT[4])
		: pos(PX, PY, PZ), normal(NX, NY, NZ), texcoord(U, V)
	{
		std::memcpy(bone, BONE, sizeof(bone));
		std::memcpy(weight, WEIGHT, sizeof(weight));
	}

	inline static void BindAttributes(GLuint program, const VertexPNTW* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNTW), offset_of(&VertexPNTW::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNTW), offset_of(&VertexPNTW::normal), baseptr);
		Vertex::EnableAttrib(program, "a_texcoord", 2, GL_FLOAT, sizeof(VertexPNTW), offset_of(&VertexPNTW::texcoord), baseptr);

		// Bone IDs (整数属性: uvec4)
		Vertex::EnableAttrib(program, "a_bone", 4, GL_UNSIGNED_INT, sizeof(VertexPNTW), offset_of(&VertexPNTW::bone), baseptr);

		// Bone Weights (float属性: vec4)
		Vertex::EnableAttrib(program, "a_weight", 4, GL_FLOAT, sizeof(VertexPNTW), offset_of(&VertexPNTW::weight), baseptr);
	}
};

struct VertexPNCTAW : public Vertex
{
public:
	static const int NUM_BONES_PER_VERTEX = 4;
	vec3 pos;
	vec3 normal;
	vec4 color;
	vec2 texcoord;
	vec3 tangent;
	uint32_t bone[NUM_BONES_PER_VERTEX];
	float weight[NUM_BONES_PER_VERTEX];
	inline void Reset()
	{
		*this = VertexPNCTAW();
	}
	inline void ResetBoneWeight()
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	inline bool AddBoneWeight(uint32_t BoneID, float Weight)
	{
//		static_assert(_countof(bone) == _countof(weight));
		for (auto i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (weight[i] == 0.0f)
			{
				bone[i] = BoneID;
				weight[i] = Weight;
				return true;
			}
		}
		return false;
	}
	inline void SortBoneWeight()
	{
//		static_assert(_countof(bone) == _countof(weight));
		for (auto i = 0; i < NUM_BONES_PER_VERTEX - 1; i++)
		{
			for (auto j = i + 1; j < NUM_BONES_PER_VERTEX; j++)
			{
				if (weight[i] < weight[j])
				{
					std::swap(weight[i], weight[j]);
					std::swap(bone[i], bone[j]);
				}
			}
		}
	}
	VertexPNCTAW(const VertexPNCTAW& src) : pos(src.pos), normal(src.normal), color(src.color), texcoord(src.texcoord), tangent(src.tangent)
	{
		std::memcpy(bone, src.bone, sizeof(bone));
		std::memcpy(weight, src.weight, sizeof(weight));
	}
	VertexPNCTAW() : pos(1.0f), normal(1.0f), color(1.0f), texcoord(0.0f), tangent(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNCTAW(const vec3& P) : pos(P), normal(0.0f), color(1.0f), texcoord(0.0f), tangent(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNCTAW(const VertexPC& PC) : pos(PC.pos), normal(0.0f), color(PC.color), texcoord(0.0f), tangent(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNCTAW(const VertexPNC& PNC) : pos(PNC.pos), normal(PNC.normal), color(PNC.color), texcoord(0.0f), tangent(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNCTAW(const VertexPNCT& PNCT) : pos(PNCT.pos), normal(PNCT.normal), color(PNCT.color), texcoord(PNCT.texcoord), tangent(0.0f)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNCTAW(const vec3& P, const vec3& N, const vec4& C, const vec2& T, const vec3& A, const uint32_t BONE[4], const float WEIGHT[4]) : pos(P), normal(N), color(C), texcoord(T), tangent(A)
	{
		std::memcpy(bone, BONE, sizeof(bone));
		std::memcpy(weight, WEIGHT, sizeof(weight));
	}
	VertexPNCTAW(const vec3& P, const vec3& N, const vec4& C, const vec2& T, const vec3& A) : pos(P), normal(N), color(C), texcoord(T), tangent(A)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}
	VertexPNCTAW(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& R, const float& G, const float& B, const float& A,
		const float& U, const float& V,
		const float& AX, const float& AY, const float& AZ,
		const uint32_t BONE[4], const float WEIGHT[4])
		: pos(PX, PY, PZ), normal(NX, NY, NZ), color(R, G, B, A), texcoord(U, V), tangent(AX, AY, AZ)
	{
		std::memcpy(bone, BONE, sizeof(bone));
		std::memcpy(weight, WEIGHT, sizeof(weight));
	}
	VertexPNCTAW(const float& PX, const float& PY, const float& PZ,
		const float& NX, const float& NY, const float& NZ,
		const float& R, const float& G, const float& B, const float& A,
		const float& U, const float& V,
		const float& AX, const float& AY, const float& AZ)
		: pos(PX, PY, PZ), normal(NX, NY, NZ), color(R, G, B, A), texcoord(U, V), tangent(AX, AY, AZ)
	{
		ZeroMemory(bone, sizeof(bone));
		ZeroMemory(weight, sizeof(weight));
	}


	inline static void BindAttributes(GLuint program, const VertexPNCTAW* baseptr = nullptr)
	{
		Vertex::DisableAttribAll(program);
		Vertex::EnableAttrib(program, "a_position", 3, GL_FLOAT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::pos), baseptr);
		Vertex::EnableAttrib(program, "a_normal", 3, GL_FLOAT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::normal), baseptr);
		Vertex::EnableAttrib(program, "a_color", 4, GL_FLOAT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::color), baseptr);
		Vertex::EnableAttrib(program, "a_texcoord", 2, GL_FLOAT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::texcoord), baseptr);
		Vertex::EnableAttrib(program, "a_tangent", 3, GL_FLOAT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::tangent), baseptr);

		// Bone IDs (整数属性: uvec4)
		Vertex::EnableAttrib(program, "a_bone", 4, GL_UNSIGNED_INT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::bone), baseptr);

		// Bone Weights (float属性: vec4)
		Vertex::EnableAttrib(program, "a_weight", 4, GL_FLOAT, sizeof(VertexPNCTAW), offset_of(&VertexPNCTAW::weight), baseptr);
	}
};
#pragma pack(pop)


































struct Geometry
{
	static inline void GenerateCheckerPlaneZX(std::vector<VertexPNCT>* vertices, const vec3& size, const vec3& grid, const vec4& color0, const vec4& color1)
	{
		assert(vertices);
		int32_t gridz = static_cast<int32_t>(grid.z);
		int32_t gridx = static_cast<int32_t>(grid.x);
		float wZ = size.z / grid.z;
		float wX = size.x / grid.x;

		vec4 color[2] = { color0, color1 };

		for (int32_t j = 0; j < gridz; j++)
		{
			float z1 = -size.z * 0.5f + wZ * static_cast<float>(j);
			float z2 = z1 + wZ;
			float v1 = static_cast<float>(j + 0) / grid.z;
			float v2 = static_cast<float>(j + 1) / grid.z;
			for (int32_t i = 0; i < gridx; i++)
			{
				float x1 = -size.x * 0.5f + wX * static_cast<float>(i);
				float x2 = x1 + wX;
				float u1 = static_cast<float>(i + 0) / grid.x;
				float u2 = static_cast<float>(i + 1) / grid.x;
				auto n = vec3(0.0f, 1.0f, 0.0f);

				auto v11 = VertexPNCT(vec3(x1, 0.0f, z1), n, color[(i + j) & 1], vec2(u1, v1));
				auto v12 = VertexPNCT(vec3(x2, 0.0f, z1), n, color[(i + j) & 1], vec2(u2, v1));
				auto v21 = VertexPNCT(vec3(x1, 0.0f, z2), n, color[(i + j) & 1], vec2(u1, v2));
				auto v22 = VertexPNCT(vec3(x2, 0.0f, z2), n, color[(i + j) & 1], vec2(u2, v2));

				vertices->push_back(v11);
				vertices->push_back(v12);
				vertices->push_back(v22);
				vertices->push_back(v11);
				vertices->push_back(v22);
				vertices->push_back(v21);
			}
		}
	}

	static inline void GenarateCube(std::vector<VertexPNCT>* vertices, const vec3& size)
	{
		vertices->clear();

		// A box has six faces, each one pointing in a different direction.
		constexpr int FaceCount = 6;

		static const vec3 faceNormals[FaceCount] =
		{
				{  0.0f,  0.0f, -1.0f },
				{  0.0f,  0.0f,  1.0f },
				{ -1.0f,  0.0f,  0.0f },
				{  1.0f,  0.0f,  0.0f },
				{  0.0f,  1.0f,  0.0f },
				{  0.0f, -1.0f,  0.0f },
		};

		static const vec2 texCoords[4] =
		{
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
		};

		auto tsize = size * 0.5f;

		// Create each face in turn.
		for (int i = 0; i < FaceCount; i++)
		{
			auto normal = faceNormals[i];

			// Get two vectors perpendicular both to the face normal and to each other.
			const auto basis = (i >= 4) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);

			const auto side1 = glm::cross(normal, basis);
			const auto side2 = glm::cross(normal, side1);

			vec4 col(MyMath::Abs(normal.x), MyMath::Abs(normal.y), MyMath::Abs(normal.z), 1.0f);
			auto v0 = VertexPNCT((normal - side1 - side2) * tsize, normal, col, texCoords[0]);
			auto v1 = VertexPNCT((normal - side1 + side2) * tsize, normal, col, texCoords[1]);
			auto v2 = VertexPNCT((normal + side1 + side2) * tsize, normal, col, texCoords[2]);
			auto v3 = VertexPNCT((normal + side1 - side2) * tsize, normal, col, texCoords[3]);

			(*vertices).push_back(v0);
			(*vertices).push_back(v1);
			(*vertices).push_back(v2);
			(*vertices).push_back(v0);
			(*vertices).push_back(v2);
			(*vertices).push_back(v3);
		}
	}

	// --- 修正された球体生成関数 (VertexPNCT を使用) ---
	static inline void GenerateSphere(std::vector<VertexPNCT>* vertices, float radius, int slices, int stacks)
	{
		// 既存のデータをクリア
		vertices->clear();

		if (slices == 0 || stacks < 2)
		{
			TRACE("Warning: slices = 0 or stacks < 2. No sphere generated.\n");
			return;
		}

		std::vector<float> sint1(slices + 1);
		std::vector<float> cost1(slices + 1);
		std::vector<float> sint2(stacks + 1);
		std::vector<float> cost2(stacks + 1);

		// スライス (全円) の計算
		float angle_step_slices = 2.0f * MyMath::_PAI / (float)slices;
		for (int i = 0; i <= slices; ++i)
		{
			sint1[i] = MyMath::Sin(angle_step_slices * i);
			cost1[i] = MyMath::Cos(angle_step_slices * i);
		}

		// スタック (半円) の計算
		float angle_step_stacks = MyMath::_PAI / (float)stacks;
		for (int i = 0; i <= stacks; ++i)
		{
			sint2[i] = MyMath::Sin(angle_step_stacks * i);
			cost2[i] = MyMath::Cos(angle_step_stacks * i);
		}

		// --- 頂点の生成 ---

		// 最初にすべてのユニークな頂点を生成する (中間ステップ)
		// grid_vertices を VertexPNCT 型に変更
		std::vector<std::vector<VertexPNCT>> grid_vertices(stacks + 1, std::vector<VertexPNCT>(slices + 1));

		// 北極と南極は、テクスチャUVが特殊なケース
		// 北極のテクスチャV座標は1.0、南極のテクスチャV座標は0.0
		// U座標は、極点に集約されるため、複数の値を持つことがありますが、
		// ここでは便宜上、0.0としておきます。実際の描画では極点のテクスチャは歪む可能性があります。

		// 北極の計算 (i=0 のスタック)
		// 北極は単一の点ですが、各スライスに対応するUVを生成して格納しておくと、
		// 下のループで使いやすくなります。
		// 極点ではU座標は意味を持ちませんが、Vは1.0です。
		// ただし、後で三角形を生成する際、grid_vertices[0][0] のように単一点を参照するため、
		// ここでループして複数の頂点を作る必要はありません。
		// 単一の北極点を作成し、テクスチャ座標も便宜上設定します。
		// U座標は通常、経度に比例しますが、極点ではすべての経度が一点に収束するため、
		// テクスチャをスムーズに貼るには工夫が必要です。
		// 簡単のため、北極点自体のUは0.0とし、周囲の三角形で補正します。
		grid_vertices[0][0] = VertexPNCT(
			vec3(0.0f, 0.0f, radius),
			vec3(0.0f, 0.0f, 1.0f),
			vec4(0.0f, 0.0f, 1.0f, 1.0f), // 青色
			vec2(0.0f, 1.0f) // Uは0.0 (便宜上), Vは1.0 (北極)
		);

		// 中間の緯度帯の頂点 (i=1 から stacks-1 まで)
		for (int i = 1; i < stacks; i++)
		{
			float v_coord = 1.0f - ((float)i / stacks); // V座標: 1.0 (北極側) から 0.0 (南極側) へ
			for (int j = 0; j <= slices; j++) // j=0 から slices まで (一周分の重複)
			{
				float x = cost1[j] * sint2[i];
				float y = sint1[j] * sint2[i];
				float z = cost2[i];

				float u_coord = (float)j / slices; // U座標: 0.0 から 1.0 へ (経度に比例)

				grid_vertices[i][j] = VertexPNCT(
					vec3(x * radius, y * radius, z * radius), // Position
					vec3(x, y, z),                           // Normal (単位球の座標がそのまま法線になる)
					vec4(MyMath::Abs(x), MyMath::Abs(y), MyMath::Abs(z), 1.0f), // Color
					vec2(u_coord, v_coord) // テクスチャ座標
				);
			}
		}

		// 南極の計算 (i=stacks のスタック)
		// 南極点自体のUは0.0 (便宜上), Vは0.0 (南極)
		grid_vertices[stacks][0] = VertexPNCT(
			vec3(0.0f, 0.0f, -radius),
			vec3(0.0f, 0.0f, -1.0f),
			vec4(0.0f, 0.0f, 1.0f, 1.0f), // 青色
			vec2(0.0f, 0.0f) // Uは0.0 (便宜上), Vは0.0 (南極)
		);


		// --- TRIANGLES として頂点を直接追加 ---

		// 1. 北極周囲の三角形
		// 北極の頂点 (grid_vertices[0][0]) を各三角形で使用
		for (int j = 0; j < slices; ++j)
		{
			// 北極点のデータは grid_vertices[0][0]
			// しかし、テクスチャ座標のU値は、次の2点に合わせて個別に設定する必要があります
			// ここでは、各三角形のために新しいVertexPNCTオブジェクトを作成し、UVを調整します。

			// 最初の頂点 (北極)
			// U座標は、この三角形の2つの隣接頂点の中間点や平均値にすると良いかもしれません。
			// または、極点ではUがどこでも良いという性質を利用し、テクスチャ座標の「縫い目」を目立たなくする工夫が必要です。
			// シンプルなアプローチとして、各Uスライスに合わせた北極点を作ります。
			// 複雑さを避けるため、今回は一旦単純にV=1.0でUを隣接頂点と合わせます。
			vertices->push_back(VertexPNCT(
				grid_vertices[0][0].pos,
				grid_vertices[0][0].normal,
				grid_vertices[0][0].color,
				vec2((grid_vertices[1][j].texcoord.x + grid_vertices[1][j + 1].texcoord.x) / 2.0f, // U座標は周囲の頂点の中間
					1.0f) // V座標は1.0 (北極)
			));

			// 2番目の頂点 (最初の緯度帯の次の頂点)
			vertices->push_back(grid_vertices[1][j + 1]);

			// 3番目の頂点 (最初の緯度帯の現在の頂点)
			vertices->push_back(grid_vertices[1][j]);
		}

		// 2. 中間の帯の四角形を2つの三角形に分割
		for (int i = 1; i < stacks - 1; ++i) // i=1 から stacks-2 まで
		{
			for (int j = 0; j < slices; ++j)
			{
				// 四角形の4つの頂点
				// v1 -- v2   (grid_vertices[i][j]   -- grid_vertices[i][j+1])
				// |     |
				// v4 -- v3   (grid_vertices[i+1][j] -- grid_vertices[i+1][j+1])

				// 1つ目の三角形 (v1, v4, v3)
				vertices->push_back(grid_vertices[i][j]);         // v1
				vertices->push_back(grid_vertices[i + 1][j]);       // v4
				vertices->push_back(grid_vertices[i + 1][j + 1]);     // v3

				// 2つ目の三角形 (v1, v3, v2)
				vertices->push_back(grid_vertices[i][j]);         // v1
				vertices->push_back(grid_vertices[i + 1][j + 1]);     // v3
				vertices->push_back(grid_vertices[i][j + 1]);       // v2
			}
		}

		// 3. 南極周囲の三角形
		// 南極の頂点 (grid_vertices[stacks][0]) を各三角形で使用
		for (int j = 0; j < slices; ++j) {
			// 南極点のデータは grid_vertices[stacks][0]
			// 北極と同様に、テクスチャUVを調整した南極点を作成します。
			vertices->push_back(VertexPNCT(
				grid_vertices[stacks][0].pos,
				grid_vertices[stacks][0].normal,
				grid_vertices[stacks][0].color,
				vec2((grid_vertices[stacks - 1][j].texcoord.x + grid_vertices[stacks - 1][j + 1].texcoord.x) / 2.0f, // U座標は周囲の頂点の中間
					0.0f) // V座標は0.0 (南極)
			));

			// 2番目の頂点 (最後の緯度帯の現在の頂点)
			vertices->push_back(grid_vertices[stacks - 1][j]);

			// 3番目の頂点 (最後の緯度帯の次の頂点)
			vertices->push_back(grid_vertices[stacks - 1][j + 1]);
		}

		TRACE("Sphere generated with %zu vertices.\n", vertices->size());
	}
};