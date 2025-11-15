#pragma once
#include "framework.h"
#include "Shader.h"
#include "Vertex.h"

template<typename T>
class DrawBuffer
{
public:
	DrawBuffer<T>(GLuint prog = 0, size_t reserve = 0) : prog_(prog), reserve_(reserve)
	{
	}

	~DrawBuffer<T>()
	{
		vertices_.clear();

		if (vbo_)
		{
			glDeleteBuffers(1, &vbo_);
			vbo_ = 0;
		}

		if (vao_)
		{
			glDeleteVertexArrays(1, &vao_);
			vao_ = 0;
		}
	}

	void Init(GLuint prog, size_t reserve = 0)
	{
		prog_ = prog;
		reserve_ = reserve;
	}

	void Begin(GLenum primitive)
	{
		if (!prog_)
		{
			TRACE("DeawBuffer のprogramがせっていされていないかも\n");
			ASSERT(0);
		}
		vertices_.clear();
		if (reserve_)
		{
			vertices_.reserve(reserve_);
		}
		primitive_ = primitive;
	}

	void Vertex(const std::vector<T>& verts)
	{
		vertices_.insert(vertices_.end(), verts.begin(), verts.end());
	}

	void Vertex(const T& v)
	{
		vertices_.push_back(v);
	}

	auto VertexCount() const
	{
		return vertices_.size();
	}

	void Reset()
	{
		vertices_.clear();
		if (reserve_)
		{
			vertices_.reserve(reserve_);
		}
	}

	void Redraw(GLenum primitive)
	{
		primitive_ = primitive;
		End();
	}

	void End()
	{
		if (vertices_.empty()) return;

		// 初回だけ VAO/VBO を生成＋レイアウト設定
		if (!vao_)
		{
			glGenVertexArrays(1, &vao_);
			if (!vao_)
			{
				assert(false);
			}
			glGenBuffers(1, &vbo_);
			if (!vbo_)
			{
				assert(false);
			}

			glBindVertexArray(vao_);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			if (!prog_)
			{
				TRACE("DeawBuffer のprogramがせっていされていないかも\n");
				ASSERT(0);
			}

			T::BindAttributes(prog_, nullptr);
		}

		// 毎回の描画時
		glBindVertexArray(vao_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(T),
			vertices_.data(), GL_DYNAMIC_DRAW);

		glDrawArrays(primitive_, 0, (GLsizei)vertices_.size());
	}

private:
	GLuint prog_ = 0;
	size_t reserve_ = 0;
	std::vector<T> vertices_;
	GLuint vao_ = 0, vbo_ = 0;
	GLenum primitive_ = GL_TRIANGLES;
};


