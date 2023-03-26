#include "hzpch.h"
#include "OpenGlTexture2DArray.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "Hazel/Log.h"
#include "stb_image_resize.h"

Hazel::OpenGlTexture2DArray::OpenGlTexture2DArray(const std::vector<std::string>& paths,int numMaterials)
	:m_Height(2048), m_Width(2048)
{
	GLenum InternalFormat = 0, Format = 0;

	if (paths.size() == 0)//if there are no texture paths then load a white texture and create a texture array with it
	{
		pixel_data = stbi_load("Assets/Textures/White.jpg", &m_Width, &m_Height, &channels, 0);
		if (pixel_data == nullptr)
			HAZEL_CORE_ERROR("Image not found!!");
		Resize_Image(32, 32);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_Renderid);
		glTextureStorage3D(m_Renderid, 1, GL_RGB8, 32, 32, numMaterials);

		glTextureParameteri(m_Renderid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_R, GL_REPEAT);
		//HAZEL_CORE_WARN(glGetError());

		for (int i = 0; i < numMaterials; i++) {
			if (resized_image)
				glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, GL_RGB, GL_UNSIGNED_BYTE, resized_image);
			else
				glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
		}
	}
	else
	{
		stbi_set_flip_vertically_on_load(1);
		pixel_data = stbi_load(paths[0].c_str(), &m_Width, &m_Height, &channels, 0);

		if (pixel_data == nullptr)
			HAZEL_CORE_ERROR("Image not found!!");

		//determine which format to use based on number of channels
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
		else if (channels == 1)
		{
			InternalFormat = GL_R8;
			Format = GL_RED;
		}
		else
			HAZEL_CORE_ERROR("Invalid Texture format");

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_Renderid);
		glTextureStorage3D(m_Renderid, 1, InternalFormat, 2048, 2048, paths.size());

		glGenerateTextureMipmap(m_Renderid);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_R, GL_REPEAT);
		//HAZEL_CORE_TRACE(glGetError());
	}
	for (int i = 0; i < paths.size(); i++)
	{
		stbi_set_flip_vertically_on_load(1);
		pixel_data = stbi_load(paths[i].c_str(), &m_Width, &m_Height, &channels, 0);

		if (pixel_data == nullptr)
			HAZEL_CORE_ERROR("Image not found!!");

		Resize_Image(2048, 2048);//resize the image if width,height > 100 (for the #trading application this is necessary)
							//otherwise not needed (i might resize the image if img dimension < 1080p or it will crash)
		
		if (resized_image)
		{
			glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, Format, GL_UNSIGNED_BYTE, resized_image);
			//HAZEL_CORE_ERROR(glGetError());
			stbi_image_free(resized_image);
		}
		else if (pixel_data) {

			glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, Format, GL_UNSIGNED_BYTE, pixel_data);
			//glTextureSubImage3D(m_Renderid, 1, 0, 0, i, m_Width, m_Height, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
			stbi_image_free(pixel_data);
		}
	}
	//glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

Hazel::OpenGlTexture2DArray::~OpenGlTexture2DArray()
{
	glDeleteTextures(1, &m_Renderid);
}

void Hazel::OpenGlTexture2DArray::Bind(int slot) const
{
	//glActiveTexture(GL_TEXTURE0 + slot);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, m_Renderid);
	glBindTextureUnit(slot, m_Renderid);
}

void Hazel::OpenGlTexture2DArray::UnBind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Hazel::OpenGlTexture2DArray::Resize_Image(const float& width, const float& height)
{
	if (m_Height > width && m_Width > height)//resize the 
	{
		//float ar = m_Width / m_Height;
		resized_image = new unsigned char[width * height * channels];
		stbir_resize_uint8(pixel_data, m_Width, m_Height, 0, resized_image, width, height, 0, channels);
		m_Height = height;
		m_Width = width;
	}
}
