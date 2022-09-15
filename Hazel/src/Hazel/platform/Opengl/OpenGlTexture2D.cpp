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
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Renderid);
		glTextureStorage2D(m_Renderid, 1, InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_Renderid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureSubImage2D(m_Renderid, 0, 0, 0, m_Width, m_Height, Format, GL_UNSIGNED_BYTE, pixel_data);

		if(pixel_data)
			stbi_image_free(pixel_data);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glDeleteTextures(1, &m_Renderid);
	}
	OpenGlTexture2D::OpenGlTexture2D(const unsigned int Width=1,const  unsigned int Height=1, unsigned int data= 0xffffffff)//for making a custom texture(white,black,red..)
		:m_Height(Height), m_Width(Width)//default texture is white of height and width = 1
	{
		GLenum InternalFormat = GL_RGBA8, Format = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_Renderid);
		glTextureStorage2D(m_Renderid, 1, InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_Renderid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureSubImage2D(m_Renderid, 0, 0, 0, m_Width, m_Height, Format, GL_UNSIGNED_BYTE, &data);

		glBindTexture(GL_TEXTURE_2D, 0);
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