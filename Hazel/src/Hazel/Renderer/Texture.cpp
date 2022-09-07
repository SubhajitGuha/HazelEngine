#include "hzpch.h"
#include "Texture.h"
#include "RendererAPI.h"
#include "Hazel/platform/Opengl/OpenGlTexture2D.h"
namespace Hazel {
	ref<Texture> Texture2D::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI()) {
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlTexture2D>(path);
		default:
			return nullptr;
		}
	}
}