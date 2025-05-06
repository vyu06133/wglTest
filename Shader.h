#pragma once
#include "framework.h"

class ShaderProg;

class Shader
{
public:
	static const GLenum Vert = GL_VERTEX_SHADER;
	static const GLenum Frag = GL_FRAGMENT_SHADER;
	static const GLenum Geom = GL_GEOMETRY_SHADER;
	static_assert(GL_GEOMETRY_SHADER == GL_GEOMETRY_SHADER_EXT);
private:
	friend class ShaderProg;
	GLenum shaderType_;
	GLuint shader_;
	ShaderProg* prog_;
	bool isLink_ = false;
	const char* path_;
	const wchar_t* wpath_;
public:
	GLuint GetShaderCode() { return shader_; }
	GLenum GetShaderType() { return shaderType_; }
	void SetProg(ShaderProg* prog) { prog_ = prog; }
	Shader() : shaderType_(GL_NONE), shader_(0), prog_(nullptr), isLink_(false), path_(nullptr), wpath_(nullptr)
	{
	}

	Shader(GLenum type) : shaderType_(type), shader_(0), prog_(nullptr), isLink_(false), path_(nullptr), wpath_(nullptr)
	{
	}

	~Shader()
	{
	}

	bool CompileShaderFromFile(GLenum type, const char* file);

	bool CompileShaderFromMemory(GLenum type, const std::vector<uint8_t>& src);
};

class ShaderProg
{
private:
	GLuint progId_;
	Shader* vertex_;
	Shader* fragment_;
	Shader* geometry_;
	std::unordered_map<std::string, GLint> uniformLocMap_;
public:
	bool AttachShader(Shader* s);

	bool DetachShader(Shader* s);

	ShaderProg() : progId_(0), vertex_(nullptr), fragment_(nullptr), geometry_(nullptr)
	{
	}

	~ShaderProg()
	{
	}

	GLuint CreateProgram()
	{
		progId_ = glCreateProgram();
		return progId_;
	}

	GLuint GetProgId() const
	{
		return progId_;
	}

	operator GLuint() const
	{
		return progId_;
	}

	void SetGeometryParam(GLint inType = GL_TRIANGLES, GLint outType = GL_TRIANGLES, GLint outVertexCnt = -1)
	{
		//ASSERT(!geometry_.empty());
/**
value	プリミティブの種類	glBeginに指定される値	入力頂点数
GL_POINTS	点	GL_POINTS	1
GL_LINES	ライン(エッジ)	GL_LINES,GL_LINE_STRIP,GL_LINE_LOOP	2
GL_LINES_ADJACENCY_EXT	近傍の情報を含むライン(エッジ)	GL_LINES_ADJACENCY_EXT,GL_LINE_STRIP_ADJACENCY_EXT	4
GL_TRIANGLES	三角形ポリゴン	GL_TRIANGLES,GL_TRIANGLE_STRIP,GL_TRIANGLE_FAN	3
GL_TRIANGLES_ADJACENCY_EXT	近傍の情報を含む三角形ポリゴン	GL_TRIANGLES_ADJACENCY_EXT,GL_TRIANGLE_STRIP_ADJACENCY_EXT	6

出力プリミティブ
GL_POINTS : 点．
GL_LINES : ライン(エッジ)．
GL_TRIANGLES : 三角形ポリゴン．
**/

// inTypeを入力してoutTypeを出力する
		glProgramParameteriEXT(progId_, GL_GEOMETRY_INPUT_TYPE_EXT, inType);
		glProgramParameteriEXT(progId_, GL_GEOMETRY_OUTPUT_TYPE_EXT, outType);

		// ジオメトリシェーダで出力する最大頂点数の取得と設定
		int vtxcnt = 0;
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &vtxcnt);
		if (outVertexCnt >= 0)
		{
			vtxcnt = outVertexCnt;
		}
		glProgramParameteriEXT(progId_, GL_GEOMETRY_VERTICES_OUT_EXT, vtxcnt);
	}

	bool LinkProg()
	{
		if (!progId_)
		{
			return false;
		}
		glLinkProgram(progId_);

		{
			GLint status;
			glGetProgramiv(progId_, GL_LINK_STATUS, &status);
//			if (status == GL_FALSE)
			{
				GLsizei size;
				glGetProgramiv(progId_, GL_INFO_LOG_LENGTH, &size);
				std::string log(size, 0);
				glGetProgramInfoLog(progId_, (GLsizei)log.length(), &size, (GLchar*)log.data());
				TRACE("glLinkProgram:%s\n", log.c_str());
//				_CrtDbgBreak();
//				return false;
			}
//			DumpActiveAttribute();
//			DumpActiveUniform();
		}
		

		uniformLocMap_.clear();
		return true;
	}

	void DumpActiveAttribute()
	{
		GLint written, size, location, maxLength, nAttribs;
		GLenum type;
		std::vector<GLchar> name;

		glGetProgramiv(progId_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
		glGetProgramiv(progId_, GL_ACTIVE_ATTRIBUTES, &nAttribs);

		name.resize(maxLength);

		TRACE(" Index | Name\n");
		TRACE("------------------------------------------------\n");
		for (int i = 0; i < nAttribs; i++) {
			glGetActiveAttrib(progId_, i, maxLength, &written, &size, &type, name.data());
			location = glGetAttribLocation(progId_, name.data());
			TRACE(" %-5d | %s\n", location, name.data());
		}
	}

	void DumpActiveUniform()
	{

		GLint nUniforms, size, location, maxLen;
		std::vector<GLchar> name;
		GLsizei written;
		GLenum type;

		glGetProgramiv(progId_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
		glGetProgramiv(progId_, GL_ACTIVE_UNIFORMS, &nUniforms);

		name.resize(maxLen);

		TRACE(" Location | Name\n");
		TRACE("------------------------------------------------\n");
		for (int i = 0; i < nUniforms; ++i)
		{
			glGetActiveUniform(progId_, i, maxLen, &written, &size, &type, name.data());
			location = glGetUniformLocation(progId_, name.data());
			TRACE(" %-8d | %s\n", location, name.data());
		}

	}
	void Dump()
	{
		TRACE("ProgramID:%u\n", progId_);
		if (!progId_)
		{
			return;
		}
		GLint numShader;
		glGetProgramiv(progId_, GL_ATTACHED_SHADERS, &numShader);
		TRACE("GL_ATTACHED_SHADERS = %i\n", numShader);

		GLint numAttribute;
		glGetProgramiv(progId_, GL_ACTIVE_ATTRIBUTES, &numAttribute);
		TRACE("GL_ACTIVE_ATTRIBUTES = %i\n", numAttribute);
		for (auto i = 0; i < numAttribute; i++)
		{
			char name[256];
			GLsizei nameLength = 0;
			int size = 0;
			GLenum type;
			glGetActiveAttrib(progId_, i, sizeof(name) - 1, &nameLength, &size, &type, name);
			name[nameLength] = '\0';
			TRACE("[%d] %s\n", i, name);
		}
		GLint numUniforms;
		glGetProgramiv(progId_, GL_ACTIVE_UNIFORMS, &numUniforms);
		TRACE("GL_ACTIVE_UNIFORMS = %d\n", numUniforms);
		for (auto i = 0; i < numUniforms; ++i)
		{
			GLint size;
			GLenum type;
			GLsizei nameLength = 0;
			GLchar name[256]; // uniform変数名の最大長を指定する

			glGetActiveUniform(progId_, i, sizeof(name) - 1, &nameLength, &size, &type, name);
			name[nameLength] = '\0';
			TRACE("[%d] %s\n", i, name);
		}
	}

	bool DeleteProg()
	{
		if (!progId_)
		{
			return false;
		}
			UnuseProg();
		glDeleteProgram(progId_);
		progId_ = 0;
		return true;
	}

	void UseProg()
	{
		glUseProgram(progId_);
	}

	void UnuseProg()
	{
		glUseProgram(0);
	}

	GLint FindUniformLoc(const char* Name)
	{
		GLint loc = -1;
		if (uniformLocMap_.count(Name) != 0)
		{
			loc = uniformLocMap_[Name];
			if (loc >= 0)
			{
				return loc;
			}
		}
		loc = glGetUniformLocation(progId_, Name);
		uniformLocMap_[Name] = loc;
		return loc;
	}

	bool ClearUniform(const char* Name)
	{
		if (uniformLocMap_.count(Name) != 0)
		{
			uniformLocMap_.erase(Name);
			return true;
		}
		return false;
	}

	bool UpdateUniformmat4(const char* Name, const glm::mat4& m)
	{
		GLint loc = FindUniformLoc(Name);
		if (loc >= 0)
		{
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
			return true;
		}
		return false;
	}

	bool UpdateUniformmat3(const char* Name, const glm::mat3& m)
	{
		GLint loc = FindUniformLoc(Name);
		if (loc >= 0)
		{
			glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
			return true;
		}
		return false;
	}

	//bool UpdateUniform(const char* Name, const glm::mat4& m)
	//{
	//	GLint loc = FindUniformLoc(Name);
	//	if (loc >= 0)
	//	{
	//		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
	//		return true;
	//	}
	//	return false;
	//}

	bool UpdateUniformvec3(const char* Name, const glm::vec3& v)
	{
		GLint loc = FindUniformLoc(Name);
		if (loc >= 0)
		{
			glUniform3f(loc, v.x, v.y, v.z);
			return true;
		}
		return false;
	}

	bool UpdateUniformvec4(const char* Name, const glm::vec4& v)
	{
		GLint loc = FindUniformLoc(Name);
		if (loc >= 0)
		{
			glUniform4f(loc, v.x, v.y, v.z, v.w);
			return true;
		}
		return false;
	}
	
	bool UpdateUniformf(const char* Name, const float& f)
	{
		GLint loc = FindUniformLoc(Name);
		if (loc >= 0)
		{
			glUniform1f(loc, f);
			return true;
		}
		return false;
	}
	bool UpdateUniformu(const char* Name, const unsigned int& u)
	{
		GLint loc = FindUniformLoc(Name);
		if (loc >= 0)
		{
			//glUniform1ui(loc, u);
			glUniform1i(loc, (GLint)u);
			return true;
		}
		return false;
	}
};

