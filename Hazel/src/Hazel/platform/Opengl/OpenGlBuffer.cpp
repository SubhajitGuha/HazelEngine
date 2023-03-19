#include "hzpch.h"
#include "OpenGlBuffer.h"
#include "glad/glad.h"

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

	OpenGlVertexBuffer::OpenGlVertexBuffer(size_t size,BufferStorage_Type Storage_Type)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		switch (Storage_Type)
		{
		case BufferStorage_Type::MUTABLE:
			glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
			break;
		case BufferStorage_Type::IMMUTABLE:
			glBufferStorage(GL_ARRAY_BUFFER, size, 0, flags);
			break;
		default:
			HAZEL_CORE_ERROR("Select correct storage type");
			glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
			break;
		}
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

	void* OpenGlVertexBuffer::MapBuffer(size_t size)
	{
		auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		return glMapBufferRange(GL_ARRAY_BUFFER, 0, size, flags);
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