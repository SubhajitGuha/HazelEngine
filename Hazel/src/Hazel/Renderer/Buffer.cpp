#include"hzpch.h"
#include"Buffer.h"
#include"Renderer.h"
#include"Hazel/platform/Opengl/OpenGlBuffer.h"
#include "Hazel/platform/Opengl/OpenglVertexArray.h"
namespace Hazel {
	

	VertexBuffer* VertexBuffer::Create(float* data, size_t size)
	{
		switch (Renderer::GetAPI()) {
		case GraphicsAPI::OpenGL:
			return new OpenGlVertexBuffer(data,size);
		case GraphicsAPI::None:
			HAZEL_CORE_ERROR("NOT A VALID GRAPHICS API");
			break;
		default:
			HAZEL_CORE_ERROR("GRAPHICS API DOESNOT MATCHES THE GIVEN API LIST");
		}
		return nullptr;
	}

	/// //////////////////////////////////////////////////////////////////////////

	IndexBuffer* IndexBuffer::Create(unsigned int* data, size_t size)
	{
		switch (Renderer::GetAPI()) {
		case GraphicsAPI::OpenGL:
			return new OpenGlIndexBuffer(data,size);
		case GraphicsAPI::None:
			HAZEL_CORE_ERROR("NOT A VALID GRAPHICS API");
			break;
		default:
			HAZEL_CORE_ERROR("GRAPHICS API DOESNOT MATCHES THE GIVEN API LIST");
		}
		return nullptr;
	}
	

	/// ////////////////////////////////////////////////////////////////////////////

	void BufferLayout ::push(std::string name, DataType type)
	{
		m_Elements.push_back(new BufferElements(name, type));
		Stride += GetSize(type);
	}
	unsigned int BufferLayout::GetSize(DataType type)
	{
		switch (type) {
		case DataType::Float: return sizeof(float);
		case DataType::Float2: return sizeof(float)*2;
		case DataType::Float3: return sizeof(float)*3;
		case DataType::Float4: return sizeof(float)*4;
		case DataType::Int: return sizeof(int);
		case DataType::Int2: return sizeof(int)*2;
		case DataType::Int3: return sizeof(int)*3;
		case DataType::Int4: return sizeof(int)*4;
		default:
			HAZEL_CORE_ERROR("Unidentfied Type");
		}

	}
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI()) {
		case GraphicsAPI::OpenGL:
			return new OpenGlVertexArray();
		case GraphicsAPI::None:
			HAZEL_CORE_ERROR("NOT A VALID GRAPHICS API");
			break;
		default:
			HAZEL_CORE_ERROR("GRAPHICS API DOESNOT MATCHES THE GIVEN API LIST");
		}
		return nullptr;
	}
}