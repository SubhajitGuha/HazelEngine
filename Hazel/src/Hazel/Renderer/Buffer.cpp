#include"hzpch.h"
#include"Buffer.h"
#include"Renderer.h"
#include"Hazel/platform/Opengl/OpenGlBuffer.h"

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
}