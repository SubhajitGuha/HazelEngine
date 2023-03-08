#include "hzpch.h"
#include "Shadows.h"
#include "Hazel/platform/Opengl/OpenGlShadows.h"

namespace Hazel
{
	Shadows::Shadows()
	{
	}
	Shadows::Shadows(float width, float height)
	{
	}
	Shadows::~Shadows()
	{
	}
	ref<Shadows> Shadows::Create(float width, float height)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlShadows>(width,height);
		}
	}
	ref<Shadows> Shadows::Create()
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlShadows>(2048, 2048);
		}
	}
}