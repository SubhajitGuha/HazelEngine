#pragma once
#include "Hazel/Core.h"
#include "Hazel/Renderer/Texture.h"
namespace Hazel {
	class OpenGlTexture2D :public Texture2D
	{
	public:
		OpenGlTexture2D(const std::string& path, bool bUse16BitTexture);
		OpenGlTexture2D(const unsigned int Width, const unsigned int Height, unsigned int);
		virtual ~OpenGlTexture2D();
		unsigned int GetWidth() override { return m_Width; }
		unsigned int GetHeight() override { return m_Height; }
		unsigned int GetChannels() override { return channels; }
		virtual void Bind(int slot)const override;
		virtual void UnBind()const override;
		unsigned int GetID() override { return m_Renderid; }
		unsigned short* GetTexture() override { return pixel_data_16; }//will not work as pixel_data is deleted
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
		void Create16BitTexture(const std::string& path);
		void Create8BitsTexture(const std::string& path);
		void CreateWhiteTexture();


	};
}
