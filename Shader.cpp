#include "pch.h"
#include "Shader.h"

// GLSL #includeを展開するchatGPTコード
std::vector<uint8_t> LoadShaderWithIncludes(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Failed to open shader file: " + filepath);
	}

	std::stringstream result;
	std::string line;
	std::filesystem::path basePath = std::filesystem::path(filepath).parent_path();

	while (std::getline(file, line))
	{
		if (line.compare(0, 8, "#include") == 0)
		{
			size_t start = line.find('"');
			size_t end = line.find_last_of('"');
			if (start != std::string::npos && end != std::string::npos && end > start)
			{
				std::string includeFilename = line.substr(start + 1, end - start - 1);
				std::filesystem::path includePath = basePath / includeFilename;

				// 一度だけ読み込む（再帰しない）
				std::ifstream includeFile(includePath, std::ios::binary);
				if (includeFile)
				{
					std::stringstream includeStream;
					includeStream << includeFile.rdbuf();
					result << includeStream.str();  // 挿入
				}
				else
				{
					throw std::runtime_error("Failed to open include file: " + includePath.string());
				}
			}
			else
			{
				result << line << '\n';  // 無効な形式ならそのまま書き込む
			}
		}
		else
		{
			result << line << '\n';
		}
	}

	std::string str = result.str();
	return std::vector<uint8_t>(str.begin(), str.end());  // UTF-8としてvectorに変換
}

bool Shader::CompileShaderFromFile(GLenum type, const char* file)
{
	path_ = file;
	std::vector<uint8_t> buf;
#if 1
	HRESULT hr = S_OK;
	buf = LoadShaderWithIncludes(file);
#else
	TRACE("CompileShaderFromFile(%s)\n", path_);
	auto hr = MyUtil::ReadFromFile(path_, &buf);
#endif
	puts((char*)buf.data());
	OutputDebugStringA((char*)buf.data());
	OutputDebugStringA("\n");
	if (FAILED(hr))
	{
		DBGBREAK();
		return false;
	}
	return CompileShaderFromMemory(type, buf);
}


bool Shader::CompileShaderFromMemory(GLenum type, const std::vector<uint8_t>& src)
{
	shaderType_ = type;
	ASSERT(shaderType_ == GL_VERTEX_SHADER || shaderType_ == GL_FRAGMENT_SHADER || shaderType_ == GL_GEOMETRY_SHADER);

	shader_ = glCreateShader(shaderType_);
	if (!shader_)
	{
		TRACE("glCreateShader failed.\n");
		return false;
	}
	auto source = (const GLchar*)src.data();
	auto length = (GLsizei)src.size();
	if (length > 2 && (src[0] == 0x0ef && src[1] == 0x0bb && src[2] == 0x0bf))
	{
		TRACE("BOM found.\n");
		source += 3;
		length -= 3;
	}
	glShaderSource(shader_, 1, &source, &length);
	glCompileShader(shader_);
	{
		GLint status;
		glGetShaderiv(shader_, GL_COMPILE_STATUS, &status);
		TRACE("COMPILE STATUS:%d\n", status);
//		if (status == GL_FALSE)
		{
			GLsizei size;
			glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &size);
			std::string log(size, 0);
			glGetShaderInfoLog(shader_, (GLsizei)log.length(), &size, (GLchar*)log.data());
			TRACE("COMPILE LOG:'%s'\n",log.c_str());
			if (status == GL_FALSE)
			{
				DBGBREAK();
				return false;
			}
		}
	}
#if 0
	glAttachShader(progId, shader_);
	glLinkProgram(progId);
	{
		GLint status;
		glGetProgramiv(progId, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLsizei size;
			glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &size);
			std::string log(size, 0);
			glGetProgramInfoLog(progId, (GLsizei)log.length(), &size, (GLchar*)log.data());
			TRACE_LOG("compile error\n%s\n", log.c_str());
			_CrtDbgBreak();
			return false;
		}
	}
	isLink_ = true;
	glDeleteShader(shader_);
#endif
	return true;
}


bool ShaderProg::AttachShader(Shader* s)
{
	if (s->prog_ == this)
	{
		return true;//allready attached
	}
	switch (s->GetShaderType())
	{
	default:
		ASSERT(0);
		return false;
	case GL_VERTEX_SHADER:
		vertex_ = s;
		s->SetProg(this);
		glAttachShader(GetProgId(), s->shader_);
		break;
	case GL_FRAGMENT_SHADER:
		fragment_ = s;
		s->SetProg(this);
		glAttachShader(GetProgId(), s->shader_);
		break;
	case GL_GEOMETRY_SHADER:
		geometry_ = s;
		s->SetProg(this);
		glAttachShader(GetProgId(), s->shader_);
		break;
	}
	return true;
}

bool ShaderProg::DetachShader(Shader* s)
{
	auto Detach = [&](Shader* s)
		{
			glDetachShader(GetProgId(), s->GetShaderCode());
			s->SetProg(nullptr);
		};
	if (vertex_ == s)
	{
		Detach(vertex_);
		vertex_ = nullptr;
	}
	if (fragment_ == s)
	{
		Detach(fragment_);
		fragment_ = nullptr;
	}
	if (geometry_ == s)
	{
		Detach(geometry_);
		geometry_ = nullptr;
	}
	return true;
}
