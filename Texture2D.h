#pragma once
#include "framework.h"


class Texture2D
{
protected:
	inline int GetByteCount(GLenum format)
	{
		switch (format)
		{
		case GL_RED:	return 1;
		case GL_ALPHA:	return 1;
		case GL_RGB:	return 3;
		case GL_BGR:	return 3;
		case GL_RGBA:	return 4;
		case GL_RGBA8:	return 4;
		case GL_LUMINANCE:	return 1;
		default:	ASSERT(0);	return 1;
		}
	}
/*
GL_ALPHA, GL_ALPHA4, GL_ALPHA8, GL_ALPHA12, GL_ALPHA16
GL_LUMINANCE, GL_LUMINANCE4, GL_LUMINANCE8, GL_LUMINANCE12
GL_LUMINANCE16
GL_LUMINANCE_ALPHA, GL_LUMINANCE4_ALPHA4, GL_LUMINANCE6_ALPHA2
GL_LUMINANCE8_ALPHA8, GL_LUMINANCE12_ALPHA4, GL_LUMINANCE12_ALPHA12
GL_LUMINANCE16_ALPHA16
GL_INTENSITY, GL_INTENSITY4, GL_INTENSITY8, GL_INTENSITY12, GL_INTENSITY16
GL_R3_G3_B2, GL_RGB, GL_RGB4, GL_RGB5, GL_RGB8, GL_RGB10, GL_RGB12, GL_RGB16
GL_RGBA, GL_RGBA2, GL_RGBA4, GL_RGB5_A1
GL_RGBA8, GL_RGB10_A2, GL_RGBA12, GL_RGBA16
*/
	inline const char* GetEnumString(GLenum format)
	{
		switch (format)
		{
		case GL_RED:	return "GL_RED";
		case GL_R8	:	return "GL_R8";
		case GL_GREEN:	return "GL_GREEN";
		case GL_BLUE:	return "GL_BLUE";
		case GL_ALPHA:	return "GL_ALPHA";
		case GL_BGR:	return "GL_BGR";
		case GL_RGB:	return "GL_RGB";
		case GL_RGB8:	return "GL_RGB8";
		case GL_RGBA8:	return "GL_RGBA8";
		case GL_RGBA:	return "GL_RGBA";
		case GL_LUMINANCE:	return "GL_LUMINANCE";
		default:		return "---";
		}
	}
	
	inline const char* GetErrotString(GLenum err)
	{
		switch (err)
		{
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_INVALID_ENUM:	return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:	return "GL_INVALID_VALUE";
		default:	return "UNKNOWN";
		}
	}
public:
	static Texture2D* CreateFromFile(const char* file);
	GLuint id_ = 0;
	GLuint activeTexture_ = GL_TEXTURE0;
	GLint width_ = 0;
	GLint height_ = 0;
	GLenum wrap_ = GL_CLAMP_TO_EDGE;
	GLenum filter_ = GL_LINEAR;
	GLenum format_ = GL_RGBA;
	Texture2D(const Texture2D&) = delete;
	/// <summary>
	/// 
	/// </summary>
	/// <param name="activeTexture">glActiveTextureでテクスチャユニットを選択するように動作させる</param>
	Texture2D(GLuint activeTexture) : activeTexture_(activeTexture) {}
	
	/// <summary>
	/// activeTextureオプション無しの場合カレントテクスチャユニットを使う
	/// </summary>
	Texture2D() {}
	~Texture2D() { DeleteTexture(); }

	vec2 InvSize()
	{
		ASSERT(width_ && height_);
		return vec2(1.0f / static_cast<float>(width_), 1.0f / static_cast<float>(height_));
	}

	bool TexImage(GLenum format = GL_RGBA8, int width = 2, int height = 2, GLenum imageformat = GL_RGBA8, void* image = nullptr)
	{
		width_ = width;
		height_ = height;
		format_ = format;
		
		DeleteTexture();
		
		//glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &id_);
		if (id_ == 0)
		{
			_CrtDbgBreak();
			return false;
		}
		BindTexture();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		if (image)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, imageformat, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::vector<uint8_t> temp(width_ * height_ * GetByteCount(imageformat));
			ZeroMemory(temp.data(), width_ * height_ * GetByteCount(imageformat));
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, imageformat, GL_UNSIGNED_BYTE, temp.data());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		SetParameter(GL_CLAMP_TO_EDGE, GL_LINEAR);
		//filter_ = GL_NEAREST;
		ApplyParameter();
		return true;
	}
	/// <summary>
	/// TexStorage
	/// </summary>
	/// <param name="format">GL_RGBA8/GL_RGB8</param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	bool TexStorage(GLenum format = GL_RGBA8, int width = 2, int height = 2)
	{
		GLenum err = glGetError();// GL_NO_ERROR;
		width_ = width;
		height_ = height;
		format_ = format;

		if (id_)
		{
			DeleteTexture();
		}

		//テクスチャオブジェクトの作成
		if (activeTexture_)
		{
			glActiveTexture(activeTexture_);
		}
		glGenTextures(1, &id_);
		if (id_ == 0)
		{
			_CrtDbgBreak();
			return false;
		}
		BindTexture();
		//glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		//テクスチャストレージ確保 
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexStorage2D(GL_TEXTURE_2D, 1, format_, width_, height_);
		glGenerateMipmap(GL_TEXTURE_2D);
		ApplyParameter();
		return true;
	}

	bool TexSubImage(int width, int height, GLenum imageformat, void* image)
	{
		if (activeTexture_)
		{
			glActiveTexture(activeTexture_);
		}
		glBindTexture(GL_TEXTURE_2D, id_);
		PAUSE(width <= width_);
		PAUSE(height <= height_);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, imageformat, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		return true;
	}
	bool TexSubImage(const Rect& rect, GLenum imageformat, void* image)
	{
		if (activeTexture_)
		{
			glActiveTexture(activeTexture_);
		}
		PAUSE(rect.left < width_);
		PAUSE(rect.right < width_);
		PAUSE(rect.top < height_);
		PAUSE(rect.bottom < height_);
		glBindTexture(GL_TEXTURE_2D, id_);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, rect.left, rect.top, rect.Width(), rect.Height(), imageformat, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		return true;
	}
	void SetParameter(GLenum wrap, GLenum filter)
	{
		wrap_ = wrap;
		filter_ = filter;
	}
	void ApplyParameter()
	{
		if (activeTexture_)
		{
			glActiveTexture(activeTexture_);
		}
		glBindTexture(GL_TEXTURE_2D, id_);
		//テクスチャの繰り返し方法の指定 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_);
		//テクスチャを拡大・縮小する方法の指定 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_);
	}
	void UnbindTexture()
	{
		if (activeTexture_)
		{
			glActiveTexture(activeTexture_);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void BindTexture()
	{
		if (activeTexture_)
		{
			glActiveTexture(activeTexture_);
		}
		glBindTexture(GL_TEXTURE_2D, id_);
	}
	void DeleteTexture()
	{
		if (id_)
		{
			if (activeTexture_)
			{
				glActiveTexture(activeTexture_);
			}
			glBindTexture(GL_TEXTURE_2D, id_);
			glDeleteTextures(1, &id_);
			id_ = 0;
		}
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="unit">GL_TEXTURE0～</param>
	void SetActiveTexture(GLuint activeTexture)
	{
		activeTexture_ = activeTexture;
	}

	/// <summary>
	/// シェーダのSampler2Dユニフォームに渡す用
	/// </summary>
	GLuint TextureUnits() const
	{
		if (activeTexture_)
		{
			return activeTexture_ - GL_TEXTURE0;
		}
		return 0;
	}

	bool LoadFile(const char* file);
	bool SaveFile(const char* file);
	bool SaveFile(const wchar_t* file);
	bool GetTexImage(std::vector<uint8_t>* image);
};

