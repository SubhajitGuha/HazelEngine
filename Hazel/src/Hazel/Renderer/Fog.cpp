#include "hzpch.h"
#include "Fog.h"
#include "Hazel/platform/Opengl/OpenGlFog.h"

namespace Hazel
{
	ref<Fog> Fog::Create(float density, float gradient, float fogStart, float fogEnd, glm::vec2 ScreenSize)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlFog>(density, gradient, fogStart, fogEnd, ScreenSize);
		default:
			return nullptr;
		}
	}
}