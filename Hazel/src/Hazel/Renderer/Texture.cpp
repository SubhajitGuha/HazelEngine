#include "hzpch.h"
#include "Texture.h"
#include "stb_image.h"
#include "RendererAPI.h"
#include "Hazel/platform/Opengl/OpenGlTexture2D.h"
#include "Hazel/platform/Opengl/OpenGlTexture2DArray.h"

namespace Hazel {
	bool Texture2D::ValidateTexture(const std::string& path)
	{
		int channels;
		int m_Height, m_Width;
		stbi_uc* pixel_data = stbi_load(path.c_str(), &m_Width, &m_Height, &channels, 0);
		if (pixel_data == nullptr)
			return false;
		else
			return true;
	}
	ref<Texture2D> Texture2D::Create(const std::string& path,bool bUse16BitTexture)
	{
		switch (RendererAPI::GetAPI()) {
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlTexture2D>(path,bUse16BitTexture);
		default:
			return nullptr;
		}
	}
	ref<Texture2D> Texture2D::Create(const unsigned int Width,const unsigned int Height,const unsigned int data)
	{
		switch (RendererAPI::GetAPI()) {
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlTexture2D>(Width,Height,data);
		default:
			return nullptr;
		}
	}
	ref<Texture2DArray> Texture2DArray::Create(const std::vector<std::string>& paths, int numMaterials, bool bUse16BitTexture)
	{
		switch (RendererAPI::GetAPI()) {
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlTexture2DArray>(paths, numMaterials,bUse16BitTexture);
		default:
			return nullptr;
		}
	}
}