#include "hzpch.h"
#include "OpenGlTexture2D.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "Hazel/Log.h"
namespace Hazel {
	OpenGlTexture2D::OpenGlTexture2D(const std::string& path)
		:m_Height(0),m_Width(0)
	{
		int channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* pixel_data = stbi_load(path.c_str(), &m_Width, &m_Height, &channels,0);

		if (pixel_data == nullptr)
			HAZEL_CORE_ERROR("Image not found!!");

		//determine which format to use based on number of channels
		GLenum InternalFormat = 0,Format = 0;
		if (channels == 4)
		{
			InternalFormat = GL_RGBA8;
			Format = GL_RGBA;
		}
		else if (channels == 3)
		{
			InternalFormat = GL_RGB8;
			Format = GL_RGB;
		}
		else
			HAZEL_CORE_ERROR("Invalid Texture format");
		
		glGenTextures(1, &m_Renderid);
		//glCreateTextures(GL_TEXTURE_2D, 1, &m_Renderid);

		//glTexStorage2D(GL_TEXTURE_2D, 0, InternalFormat, m_Width, m_Height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, Format, GL_UNSIGNED_BYTE, pixel_data);
		glTexImage2D(GL_TEXTURE_2D, 0,InternalFormat, m_Width, m_Height, 0, Format, GL_UNSIGNED_BYTE, pixel_data);

		if(pixel_data)
			stbi_image_free(pixel_data);
	}
	OpenGlTexture2D::~OpenGlTexture2D()
	{
		glDeleteTextures(1, &m_Renderid);
	}
	void OpenGlTexture2D::Bind(int slot = 0) const
	{
		glBindTextureUnit(slot, m_Renderid);
	}
	void OpenGlTexture2D::UnBind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}