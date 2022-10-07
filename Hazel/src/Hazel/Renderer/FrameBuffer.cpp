#include "hzpch.h"
#include "FrameBuffer.h"
#include "RendererAPI.h"
#include "Hazel/platform/Opengl/OpenGlFrameBuffer.h"

namespace Hazel {
    ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
    {
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlFrameBuffer>(spec);
		default:
			return nullptr;
		}
    }
}