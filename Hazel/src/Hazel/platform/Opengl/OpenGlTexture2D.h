#pragma once
#include "Hazel/Core.h"
#include "Hazel/Renderer/Texture.h"
namespace Hazel {
	class OpenGlTexture2D :public Texture2D
	{
	public:
		OpenGlTexture2D(const std::string& path);
		OpenGlTexture2D(const unsigned int Width, const unsigned int Height, unsigned int);
		virtual ~OpenGlTexture2D();
		unsigned int GetWidth() override { return m_Width; }
		unsigned int GetHeight() override { return m_Height; }
		virtual void Bind(int slot)const override;
		virtual void UnBind()const override;
	private:
		 int m_Width;
		 int m_Height;
		unsigned int m_Renderid;
	};
}
