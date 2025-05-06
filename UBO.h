#pragma once

template <typename T>
class UBO
{
public:
	UBO()
		: m_data(), m_uboID(0)
	{
	}
	
	~UBO()
	{
		if (m_uboID != 0)
		{
			glDeleteBuffers(1, &m_uboID);
		}
	}

	GLuint GenAndBind(GLuint program, const GLchar* uniformBlockName)
	{
		glGenBuffers(1, &m_uboID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		Bind(program, uniformBlockName);
		return m_uboID;
	}

	GLuint GenAndBind(GLuint program, const GLuint bindingPoint)
	{
		glGenBuffers(1, &m_uboID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		Bind(program, bindingPoint);
		return m_uboID;
	}

	// UBOをブロック名でバインドする
	void Bind(GLuint program, const GLchar* uniformBlockName)
	{
		//BUG:名前ベースの方式がうまく行かない
		auto blockIndex = glGetUniformBlockIndex(program, uniformBlockName);
		if (blockIndex == GL_INVALID_INDEX)
		{
			TRACE("uniformブロック'%s'が無い\n", uniformBlockName);
			DBGBREAK();
		}
		glUniformBlockBinding(program, blockIndex, blockIndex);

		// バインドポイントとUBOを関連付け
		glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, m_uboID);
	}

	// UBOをGLSL430記述’layout(std140,binding=N)’のNでバインドする
	void Bind(GLuint program, const GLuint bindingPoint) const
	{
		//MEMO:未検証
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_uboID);
	}

	// UBOにデータを更新する
	void SendToGPU(const T* data = nullptr)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0/*ofs*/, sizeof(T), data ? data : &m_data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	GLuint GetID() const
	{
		return m_uboID;
	}

	T& Data()
	{
		return m_data;
	}

private:
	T m_data;
	GLuint m_uboID;
};

