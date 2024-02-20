#pragma once
#include "Hazel/Core.h"
#include "Hazel/Renderer/Texture.h"
namespace Hazel {
	class OpenGlTexture2DArray :public Texture2DArray
	{
	public:
		OpenGlTexture2DArray(const std::vector<std::string>& paths, int numMaterials, int numChannels, bool bUse16BitTexture );
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
		unsigned short* resized_image_16 = nullptr;
		unsigned short* pixel_data_16 = nullptr;
		unsigned char* resized_image_8 = nullptr;
		unsigned char* pixel_data_8 = nullptr;
	private:
		void Resize_Image(const float& width, const float& height, bool bUse16BitTexture = false);
		void Create16BitTextures(const std::vector<std::string>& paths, int numMaterials);
		void Create8BitsTextures(const std::vector<std::string>& paths, int numMaterials);
		void CreateWhiteTextureArray(int numMat);
	};
}
