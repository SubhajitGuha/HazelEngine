#include "hzpch.h"
#include "OpenGlBuffer.h"
#include"glad/glad.h"

namespace Hazel {

	////////////////////////////////////////////////////////
	/// Vertex Buffer //////////////////////////////////////
	//////////////////////////////////////////////////////

	OpenGlVertexBuffer::OpenGlVertexBuffer(float* data, size_t size)
	{
		glGenBuffers(1, &m_RendererID);//set up vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	OpenGlVertexBuffer::OpenGlVertexBuffer(size_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGlVertexBuffer::~OpenGlVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGlVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGlVertexBuffer::UnBind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGlVertexBuffer::SetData(size_t size, const void* data)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0,size,data);
	}
	
	////////////////////////////////////////////////////////
	/// Index Buffer //////////////////////////////////////
	//////////////////////////////////////////////////////
	

	OpenGlIndexBuffer::OpenGlIndexBuffer(unsigned int* data, size_t size)
	{
		m_Elements = size/sizeof(unsigned int);
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}
	OpenGlIndexBuffer::~OpenGlIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGlIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}
	void OpenGlIndexBuffer::UnBind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	size_t OpenGlIndexBuffer::GetCount()
	{
		return m_Elements;
	}
}