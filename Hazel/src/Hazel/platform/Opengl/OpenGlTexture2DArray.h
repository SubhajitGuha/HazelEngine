#pragma once
#include "Hazel/Core.h"
#include "Hazel/Renderer/Texture.h"
namespace Hazel {
	class OpenGlTexture2DArray :public Texture2DArray
	{
	public:
		OpenGlTexture2DArray(const std::vector<std::string>& paths, int numMaterials);
		~OpenGlTexture2DArray();
		unsigned int GetWidth() override { return m_Width; }
		unsigned int GetHeight() override { return m_Height; }
		virtual void Bind(int slot)const override;
		virtual void UnBind()const override;
		unsigned int GetID() override { return m_Renderid; }
	private:
		int m_Width;
		int m_Height;
		int channels;
		unsigned int m_Renderid;
		unsigned short* resized_image = nullptr;
		unsigned short* pixel_data = nullptr;
	private:
		void Resize_Image(const float& width, const float& height);
		void CreateWhiteTextureArray(int numMat);
	};
}
