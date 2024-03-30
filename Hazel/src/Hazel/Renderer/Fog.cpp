#include "hzpch.h"
#include "Fog.h"
#include "Hazel/platform/Opengl/OpenGlFog.h"

namespace Hazel
{
	ref<Fog> Fog::Create(float density, float fogStart, float fogEnd, float fogTop, float fogBottom, glm::vec2 ScreenSize)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlFog>(density, fogStart, fogEnd, fogTop, fogBottom, ScreenSize);
		default:
			return nullptr;
		}
	}
}