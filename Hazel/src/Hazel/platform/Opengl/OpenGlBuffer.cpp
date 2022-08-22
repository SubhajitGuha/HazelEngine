#include "hzpch.h"
#include "OpenGlBuffer.h"
#include"glad/glad.h"

namespace Hazel {

	////////////////////////////////////////////////////////
	/// Vertex Buffer //////////////////////////////////////
	//////////////////////////////////////////////////////

	OpenGlVertexBuffer::OpenGlVertexBuffer(float* data, size_t size)
	{
		glGenBuffers(1, &m_Renderer);//set up vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_Renderer);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	OpenGlVertexBuffer::~OpenGlVertexBuffer()
	{
		glDeleteBuffers(1, &m_Renderer);
	}

	void OpenGlVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_Renderer);
	}

	void OpenGlVertexBuffer::UnBind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	////////////////////////////////////////////////////////
	/// Index Buffer //////////////////////////////////////
	//////////////////////////////////////////////////////
	

	OpenGlIndexBuffer::OpenGlIndexBuffer(unsigned int* data, size_t size)
	{
		glGenBuffers(1, &m_Renderer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Renderer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}
	OpenGlIndexBuffer::~OpenGlIndexBuffer()
	{
		glDeleteBuffers(1, &m_Renderer);
	}
	void OpenGlIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Renderer);
	}
	void OpenGlIndexBuffer::UnBind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}