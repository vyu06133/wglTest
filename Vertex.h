#pragma once
#include "framework.h"
#include "MyMath.h"

struct VAO_INPUT_ELEMENT_DESC
{
	const char* SemanticName;
	GLint index;
	GLint size;
	GLenum type;
};

class VertexP
{
public:
	vec3 pos;
	VertexP() :
		pos(0.0f)
	{}
	VertexP(const vec3& P) :
		pos(P)
	{}
	VertexP(const float& x, const float& y, const float& z) :
		pos(x, y, z)
	{}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		pos = vec3(0.0f);
	}
	inline void Command() const
	{
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPN : public VertexP
{
public:
	vec3 normal;
	VertexPN() :
		VertexP(), normal(0.0f)
	{}
	VertexPN(const vec3& P, const vec3& N) :
		VertexP(P), normal(N)
	{}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		normal = vec3(0.0f);
	}
	inline void Command() const
	{
		glNormal3fv(glm::value_ptr(normal));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPT : public VertexP
{
public:
	vec2 tex;
	VertexPT() :
		VertexP(), tex(0.0f)
	{
	}
	VertexPT(const float& x, const float& y,const float& z, const float& s, const float& t) :
		VertexP(x, y, z), tex(s, t)
	{
	}
	VertexPT(const vec3& P, const vec2& T) :
		VertexP(P), tex(T)
	{
	}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		tex = vec2(0.0f);
	}
	inline void Command() const
	{
		glTexCoord2fv(glm::value_ptr(tex));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPC : public VertexP
{
public:
	vec4 color;
	VertexPC() :
		VertexP(), color(1.0f)
	{}
	VertexPC(const vec3& P, const vec4& C) :
		VertexP(P), color(C)
	{}
	VertexPC(
		const float& x, const float& y, const float& z,
		const float& r, const float& g, const float& b, const float& a) :
		VertexP(x, y, z), color(r, g, b, a)
	{}
	
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		color = vec4(0.0f);
	}
	inline void Command() const
	{
		glColor4fv(glm::value_ptr(color));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPNC : public VertexPN
{
public:
	vec4 color;
	VertexPNC() :
		VertexPN(), color(1.0f)
	{}
	VertexPNC(const vec3& P, const vec3& N, const vec4& C) :
		VertexPN(P, N), color(C)
	{}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		color = vec4(0.0f);
	}
	inline void Command() const
	{
		glColor4fv(glm::value_ptr(color));
		glNormal3fv(glm::value_ptr(normal));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPNT : public VertexPN
{
public:
	vec2 tex;
	VertexPNT() :
		VertexPN(), tex(0.0f)
	{}
	VertexPNT(const vec3& P, const vec3& N, const vec2& T) :
		VertexPN(P, N), tex(T)
	{}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		tex = vec2(0.0f);
	}
	inline void Command() const
	{
		glTexCoord2fv(glm::value_ptr(tex));
		glNormal3fv(glm::value_ptr(normal));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPCT : public VertexPC
{
public:
	vec2 tex;
	VertexPCT() :
		VertexPC(), tex(0.0f)
	{}
	VertexPCT(const vec3& P, const vec4& C, const vec2& T) :
		VertexPC(P, C), tex(T)
	{}
	VertexPCT(
		const float& x, const float& y, const float& z,
		const float& r, const float& g, const float& b, const float& a,
		const float& u, const float& v) :
		VertexPC(x,y,z, r,g,b,a), tex(u,v)
	{}
	VertexPCT(const VertexPT& pt, const vec4& C=vec4(1.0f)) :
		VertexPC(pt.pos, C), tex(pt.tex)
	{
	}
	VertexPCT(const vec3& P, const vec4& C, const float& u, const float& v) :
		VertexPC(P, C), tex(u, v)
	{
	}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		tex = vec2(0.0f);
	}
	inline void Command() const
	{
		glTexCoord2fv(glm::value_ptr(tex));
		glColor4fv(glm::value_ptr(color));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPNCT : public VertexPNC
{
public:
	vec2 tex;
	VertexPNCT() :
		VertexPNC(), tex(0.0f)
	{}
	VertexPNCT(const VertexPC& PC) : VertexPNC(PC.pos, vec3(), PC.color), tex()
	{}
	VertexPNCT(const vec3& P, const vec3& N, const vec4& C, const vec2& T) :
		VertexPNC(P, N, C), tex(T)
	{}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		tex = vec2(0.0f);
	}
	inline void Command() const
	{
		glTexCoord2fv(glm::value_ptr(tex));
		glColor4fv(glm::value_ptr(color));
		glNormal3fv(glm::value_ptr(normal));
		glVertex3fv(glm::value_ptr(pos));
	}
};

class VertexPNCTA : public VertexPNCT
{
public:
	vec3 tangent;
	VertexPNCTA() :
		VertexPNCT(), tangent(0.0f)
	{}
	VertexPNCTA(const vec3& P, const vec3& N, const vec4& C, const vec2& T, const vec3& A) :
		VertexPNCT(P, N, C, T), tangent(A)
	{}
	inline static VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{ "TANGENT", 4, 3, GL_FLOAT },
		{},
	};
	inline void Reset()
	{
		__super::Reset();
		tangent = vec3(0.0f);
	}
	inline void Command() const
	{
		ASSERT(0);	// 頂点TANGENTを使う場合頂点バッファオブジェクトとGLSLシェーダーを使う前提
		glTexCoord3fv(glm::value_ptr(tangent));
		glTexCoord2fv(glm::value_ptr(tex));
		glColor4fv(glm::value_ptr(color));
		glNormal3fv(glm::value_ptr(normal));
		glVertex3fv(glm::value_ptr(pos));
	}
};


struct BoneWeight
{
	static const int NUM_BONES_PER_VERTEX = 4;
	glm::u8vec4 IDs;
	glm::vec4 Weights;

	BoneWeight()
	{
		Reset();
	};

	void Reset()
	{
		IDs = glm::u8vec4(0u);
		Weights = glm::vec4(0.0f);
	}
	
	bool operator==(const BoneWeight& rhs)
	{
		return IDs == rhs.IDs && Weights == rhs.Weights;
	}

	bool AddBoneData(uint32_t BoneID, float Weight)
	{
		for (auto i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (Weights[i] == 0.0f)
			{
				IDs[i] = (uint8_t)BoneID;
				Weights[i] = Weight;
				return true;
			}
		}
		return false;
	}

	void Sort(int numBones = NUM_BONES_PER_VERTEX)
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
		if (numBones < NUM_BONES_PER_VERTEX)
		{
			for (auto i = numBones; i < NUM_BONES_PER_VERTEX; i++)
			{
				Weights[i] = 0.0f;
			}
			float s = Weights[0] + Weights[1] + Weights[2] + Weights[3];//glm::sum(Weights);
			if (s)
			{
				Weights[0] /= s;
				Weights[1] /= s;
				Weights[2] /= s;
				Weights[3] /= s;
			}
		}
	}
};

class VertexPNCW : public VertexPNC
{
public:
	BoneWeight weight;
	VertexPNCW() :
		VertexPNC(), weight()
	{
	}
	VertexPNCW(const vec3& P, const vec3& N, const vec4& C, const BoneWeight& W = BoneWeight()) :
		VertexPNC(P, N, C), weight(W)
	{
	}
	inline void Reset()
	{
		__super::Reset();
		weight.Reset();
	}
	inline static const  VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{ "BLENDINDICES", 5, 4, GL_UNSIGNED_INT },
		{ "BLENDWEIGHT", 6, 4, GL_FLOAT },
		{},
	};
};

class VertexPNTW : public VertexPNT
{
public:
	BoneWeight weight;
	VertexPNTW() :
		VertexPNT(), weight()
	{
	}
	VertexPNTW(const vec3& P, const vec3& N, const vec2& T, const BoneWeight& W = BoneWeight()) :
		VertexPNT(P, N, T), weight(W)
	{
	}
	inline void Reset()
	{
		__super::Reset();
		weight.Reset();
	}
	inline static const  VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{ "BLENDINDICES", 5, 4, GL_UNSIGNED_BYTE },
		{ "BLENDWEIGHT", 6, 4, GL_FLOAT },
		{},
	};
};

class VertexPNCTW : public VertexPNCT
{
public:
	BoneWeight weight;
	VertexPNCTW() :
		VertexPNCT(), weight()
	{
	}
	VertexPNCTW(const vec3& P, const vec3& N, const vec4& C, const vec2& T, const BoneWeight& W = BoneWeight()) :
		VertexPNCT(P, N, C, T), weight(W)
	{
	}
	inline void Reset()
	{
		__super::Reset();
		weight.Reset();
	}
	inline static const  VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{ "BLENDINDICES", 5, 4, GL_UNSIGNED_BYTE },
		{ "BLENDWEIGHT", 6, 4, GL_FLOAT },
		{},
	};
};

class VertexPNCTAW : public VertexPNCTA
{
public:
	BoneWeight weight;
	VertexPNCTAW() :
		VertexPNCTA(), weight()
	{
	}
	VertexPNCTAW(const VertexPNCTA& PNCTA) :
		VertexPNCTA(PNCTA), weight()
	{
	}
	VertexPNCTAW(const vec3& P, const vec3& N, const vec4& C, const vec2& T, const vec3& A, const BoneWeight& W = BoneWeight()) :
		VertexPNCTA(P, N, C, T, A), weight(W)
	{
	}
	inline void Reset()
	{
		__super::Reset();
		weight.Reset();
	}
	inline static const  VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "NORMAL", 1, 3, GL_FLOAT },
		{ "COLOR", 2, 4, GL_FLOAT },
		{ "TEXCOORD", 3, 2, GL_FLOAT },
		{ "TANGENT", 4, 3, GL_FLOAT },
		{ "BLENDINDICES", 5, 4, GL_UNSIGNED_BYTE },
		{ "BLENDWEIGHT", 6, 4, GL_FLOAT },
		{},
	};
};

class VertexPW : public VertexP
{
public:
	BoneWeight weight;
	VertexPW() :
		VertexP(), weight()
	{
	}
	VertexPW(const VertexP& P) :
		VertexP(P), weight()
	{
	}
	VertexPW(const vec3& P, const BoneWeight& W = BoneWeight()) :
		VertexP(P), weight(W)
	{
	}
	inline void Reset()
	{
		__super::Reset();
		weight.Reset();
	}
	inline static const  VAO_INPUT_ELEMENT_DESC InputLayout[] = {
		{ "POSITION", 0, 3, GL_FLOAT },
		{ "BLENDINDICES", 5, 4, GL_UNSIGNED_BYTE },
		{ "BLENDWEIGHT", 6, 4, GL_FLOAT },
		{},
	};
};


inline static int GetBytesizeFromGLType(GLenum type)
{
	switch (type)
	{
	case GL_BYTE: 			return 1;
	case GL_UNSIGNED_BYTE: 	return 1;
	case GL_SHORT: 			return 2;
	case GL_UNSIGNED_SHORT: return 2;
	case GL_INT: 			return 4;
	case GL_UNSIGNED_INT: 	return 4;
	case GL_HALF_FLOAT: 	return 2;
	case GL_FLOAT: 			return 4;
	case GL_CLAMP: 			return 4;
	case GL_DOUBLE: 		return 8;
	default:	_CrtDbgBreak();	return 0;
	}
}

template <typename T>
class VertexBuffer
{
public:
	inline static void ClearAttribPointer(int IndexRange=8)
	{
		for (auto i = 0; i < IndexRange; i++)
		{
			glEnableVertexAttribArray(i);
		}
	}
	inline static void EnableAttribPointer()
	{
		VAO_INPUT_ELEMENT_DESC* elm = (VAO_INPUT_ELEMENT_DESC*)T::InputLayout;
		while (elm && elm->SemanticName != nullptr)
		{
			glEnableVertexAttribArray(elm->index);
			elm++;
		}
	}
	inline static void DisableAttribPointer()
	{
		VAO_INPUT_ELEMENT_DESC* elm = (VAO_INPUT_ELEMENT_DESC*)T::InputLayout;
		while (elm && elm->SemanticName != nullptr)
		{
			glDisableVertexAttribArray(elm->index);
			elm++;
		}
	}
	inline static void VertexAttribPointer()
	{
		VAO_INPUT_ELEMENT_DESC* elm = (VAO_INPUT_ELEMENT_DESC*)T::InputLayout;
		size_t ofs = 0;
		while (elm && elm->SemanticName != nullptr)
		{
			glEnableVertexAttribArray(elm->index);
			TRACE("VertexAttribPointer(index=%d, size=%d, %s, false, strride=%d, ofs=%d) '%s'\n",
				elm->index, elm->size, glEnumMap[elm->type], sizeof(T), (GLvoid*)ofs, elm->SemanticName);
			auto t = elm->type;
			if (t == GL_BYTE || t == GL_UNSIGNED_BYTE || t == GL_SHORT || t == GL_UNSIGNED_SHORT || t == GL_INT || t == GL_UNSIGNED_INT)
			{
				glVertexAttribIPointer(elm->index, elm->size, elm->type, sizeof(T), (GLvoid*)ofs);
			}
			else
			{
				glVertexAttribPointer(elm->index, elm->size, elm->type, GL_FALSE, sizeof(T), (GLvoid*)ofs);
			}
			ofs += GetBytesizeFromGLType(elm->type) * elm->size;
			elm++;
		}
	}
private:
	GLuint vaoID_, vboID_; // VAOとVBOのID
	GLsizei vertexCount_; // 頂点数
	GLuint usage_;

public:
	VertexBuffer() : vaoID_(0), vboID_(0), vertexCount_(0), usage_(GL_DYNAMIC_DRAW)
	{
	}
	~VertexBuffer()
	{
		Delete();
	}

	bool Setup(const std::vector<T>& vertices, GLenum usage = GL_DYNAMIC_DRAW)
	{
		PAUSE(vertices.size());
		return Setup(vertices.data(), vertices.size(), usage);
	}
	bool Setup(const T* data, size_t size, GLenum usage = GL_DYNAMIC_DRAW)
	{
		//ASSERT(data);
		ASSERT(size);
		usage_ = usage;
		ASSERT(usage == GL_STATIC_DRAW || usage == GL_DYNAMIC_DRAW);
		//if (data == nullptr || size == 0)
		//{
		//	return false;
		//}
		if (vaoID_ == 0)
		{
			glGenVertexArrays(1, &vaoID_);
			if (vaoID_ == 0)
			{
				return false; // VAO生成失敗
			}
		}

		if (vboID_ == 0)
		{
			glGenBuffers(1, &vboID_);
			if (vboID_ == 0)
			{
				return false; // VBO生成失敗
			}
		}
		//if (!vaoID_ || !vboID_)
		//{
		//	glGenVertexArrays(1, &vaoID_);
		//	glGenBuffers(1, &vboID_);
		//	//glGenVertexArrays や glGenBuffers のエラーチェックがない。
		//}
		ASSERT(vaoID_);
		ASSERT(vboID_);
		vertexCount_ = static_cast<GLsizei>(size);
		glBindVertexArray(vaoID_);
		glBindBuffer(GL_ARRAY_BUFFER, vboID_);
			//glBufferData でデータ転送が失敗する可能性があるので、glGetError() などで確認するとより安全。
		if (usage == GL_STATIC_DRAW)
		{
			glBufferData(GL_ARRAY_BUFFER, size * sizeof(T), data, usage);
		}
		else//if (usage == GL_DYNAMIC_DRAW)
		{
			glBufferData(GL_ARRAY_BUFFER, size * sizeof(T), data, usage);
		}
		vertexCount_ = static_cast<GLsizei>(size);

		// 頂点属性
		VertexAttribPointer();

		// バインド解除（安全のため）
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return true;
	}

	void Bind() const
	{
		glBindVertexArray(vaoID_);
	}

	void Unbind()
	{
		glBindVertexArray(0);
	}

	// データを更新
	void UpdateData(const std::vector<GLfloat>& vertices)
	{
		ASSERT(usage_ != GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vboID_);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());
	}

	void UpdateData(const std::vector<T>& vertices)
	{
		ASSERT(usage_ != GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vboID_);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(T), vertices.data());
		vertexCount_ = static_cast<GLsizei>(vertices.size());
	}

	void UpdateData(const T* vertices, size_t size)
	{
		ASSERT(usage_ != GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vboID_);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(T), vertices);
		vertexCount_ = static_cast<GLsizei>(size);
	}

	// デストラクタ: リソース解放
	void Delete()
	{
		glDeleteBuffers(1, &vboID_);
		glDeleteVertexArrays(1, &vaoID_);
	}

	GLsizei GetVertexCount() const
	{
		return vertexCount_;
	}
};

