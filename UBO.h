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

	// UBO���u���b�N���Ńo�C���h����
	void Bind(GLuint program, const GLchar* uniformBlockName)
	{
		//BUG:���O�x�[�X�̕��������܂��s���Ȃ�
		auto blockIndex = glGetUniformBlockIndex(program, uniformBlockName);
		if (blockIndex == GL_INVALID_INDEX)
		{
			TRACE("uniform�u���b�N'%s'������\n", uniformBlockName);
			DBGBREAK();
		}
		glUniformBlockBinding(program, blockIndex, blockIndex);

		// �o�C���h�|�C���g��UBO���֘A�t��
		glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, m_uboID);
	}

	// UBO��GLSL430�L�q�flayout(std140,binding=N)�f��N�Ńo�C���h����
	void Bind(GLuint program, const GLuint bindingPoint) const
	{
		//MEMO:������
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_uboID);
	}

	// UBO�Ƀf�[�^���X�V����
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

