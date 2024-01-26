#include "hzpch.h"
#include "OpenGlTexture2DArray.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "Hazel/Log.h"
#include "stb_image_resize.h"


Hazel::OpenGlTexture2DArray::OpenGlTexture2DArray(const std::vector<std::string>& paths,int numMaterials,int numChannels, bool bUse16BitTexture)
	:m_Height(0), m_Width(0)
{
	//no hash ID generation happening

	if (paths.size() == 0)//if there are no texture paths then load a white texture and create a texture array with it
	{
		CreateWhiteTextureArray(numMaterials);
	}
	else
	{
		if (bUse16BitTexture)
			Create16BitTextures(paths, numMaterials, numChannels);
		else
			Create8BitsTextures(paths, numMaterials, numChannels);
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

void Hazel::OpenGlTexture2DArray::Resize_Image(const float& width, const float& height, bool bUse16BitTexture)
{
	if (bUse16BitTexture)
	{
		if (m_Height > width && m_Width > height)//resize the 
		{
			//float ar = m_Width / m_Height;
			resized_image_16 = new unsigned short[width * height * channels];
			stbir_resize_uint16_generic(pixel_data_16, m_Width, m_Height, 0, resized_image_16, width, height, 0, channels, 0, 0, STBIR_EDGE_REFLECT, STBIR_FILTER_BOX, STBIR_COLORSPACE_LINEAR, 0);
			m_Height = height;
			m_Width = width;
		}
	}
	else
	{
		if (m_Height > width && m_Width > height)//resize the 
		{
			//float ar = m_Width / m_Height;
			resized_image_8 = new unsigned char[width * height * channels];
			stbir_resize_uint8(pixel_data_8, m_Width, m_Height, 0, resized_image_8, width, height, 0,channels);
			
			m_Height = height;
			m_Width = width;
		}
		
	}
}

void Hazel::OpenGlTexture2DArray::Create16BitTextures(const std::vector<std::string>& paths, int numMaterials, int numChannels)
{
	GLenum InternalFormat = 0, Format = 0;

	//stbi_set_flip_vertically_on_load(1);
	//pixel_data_16 = stbi_load_16(paths[0].c_str(), &m_Width, &m_Height, &channels, 0);
	//
	//if (pixel_data_16 == nullptr) {
	//	HAZEL_CORE_ERROR("2D array Image not found!!");
	//	CreateWhiteTextureArray(numMaterials);
	//}
	//else //if the image is loaded
	{
		channels = numChannels;
		//determine which format to use based on number of channels
		if (channels == 4)
		{
			InternalFormat = GL_RGBA16;
			Format = GL_RGBA;
		}
		else if (channels == 3)
		{
			InternalFormat = GL_RGB16;
			Format = GL_RGB;
		}
		else if (channels == 2)
		{
			InternalFormat = GL_RG16;
			Format = GL_RG;
		}
		else if (channels == 1)
		{
			InternalFormat = GL_R16;
			Format = GL_RED;
		}
		else
			HAZEL_CORE_ERROR("Invalid Texture format");

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_Renderid);
		glTextureStorage3D(m_Renderid, 1, InternalFormat, m_Width, m_Height, paths.size());

		glGenerateTextureMipmap(m_Renderid);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_R, GL_REPEAT);
		//HAZEL_CORE_TRACE(glGetError());

		for (int i = 0; i < paths.size(); i++)
		{
			stbi_set_flip_vertically_on_load(1);
			pixel_data_16 = stbi_load_16(paths[i].c_str(), &m_Width, &m_Height, &channels, 0);

			if (pixel_data_16 == nullptr) //if no image is found load a white texture
				pixel_data_16 = stbi_load_16("Assets/Textures/White.jpg", &m_Width, &m_Height, &channels, 0);

			//Resize_Image(2048, 2048);//resize the image if width,height > 100 (for the #trading application this is necessary)
								//otherwise not needed (i might resize the image if img dimension < 1080p or it will crash)

			if (resized_image_16)
			{
				glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, Format, GL_UNSIGNED_SHORT, resized_image_16);
				//HAZEL_CORE_ERROR(glGetError());
				stbi_image_free(resized_image_16);
			}
			else if (pixel_data_16) {

				glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, Format, GL_UNSIGNED_SHORT, pixel_data_16);
				//glTextureSubImage3D(m_Renderid, 1, 0, 0, i, m_Width, m_Height, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel_data_8);
				stbi_image_free(pixel_data_16);
			}
		}
	}

}

void Hazel::OpenGlTexture2DArray::Create8BitsTextures(const std::vector<std::string>& paths, int numMaterials, int numChannels)
{
	GLenum InternalFormat = 0, Format = 0;

	//stbi_set_flip_vertically_on_load(1);
	//pixel_data_8 = stbi_load(paths[0].c_str(), &m_Width, &m_Height, &channels, 0);

	//if (pixel_data_8 == nullptr) {
	//	HAZEL_CORE_ERROR("2D array Image not found!!");
	//	CreateWhiteTextureArray(numMaterials);
	//}
	//else //if the image is loaded
	{
		channels = numChannels;
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
		else if (channels == 2)
		{
			InternalFormat = GL_RG8;
			Format = GL_RG;
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
		for (int i = 0; i < paths.size(); i++)
		{
			stbi_set_flip_vertically_on_load(1);
			pixel_data_8 = stbi_load(paths[i].c_str(), &m_Width, &m_Height, &channels, 0);

			if (pixel_data_8 == nullptr) //if no image load the white texture
				pixel_data_8 = stbi_load("Assets/Textures/White.jpg", &m_Width, &m_Height, &channels, 0);

			//Resize_Image(2048, 2048);//resize the image if width,height > 100 (for the #trading application this is necessary)
								//otherwise not needed (i might resize the image if img dimension < 1080p or it will crash)

			if (resized_image_8)
			{
				glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, InternalFormat, m_Width, m_Height, 1, 0, Format, GL_UNSIGNED_BYTE, resized_image_8);
				//HAZEL_CORE_ERROR(glGetError());
				stbi_image_free(resized_image_16);
			}
			else if (pixel_data_8) {

				glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, Format, GL_UNSIGNED_BYTE, pixel_data_8);
				//glTextureSubImage3D(m_Renderid, 1, 0, 0, i, m_Width, m_Height, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel_data_8);
				stbi_image_free(pixel_data_8);
			}
		}	
	}
}

void Hazel::OpenGlTexture2DArray::CreateWhiteTextureArray(int numMaterials)
{
	pixel_data_8 = stbi_load("Assets/Textures/White.jpg", &m_Width, &m_Height, &channels, 0);
	if (pixel_data_8 == nullptr)
		HAZEL_CORE_ERROR("Image not found!!");
	Resize_Image(16, 16);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_Renderid);
	glTextureStorage3D(m_Renderid, 1, GL_RGB8, 16, 16, numMaterials);

	glTextureParameteri(m_Renderid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_Renderid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_Renderid, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//HAZEL_CORE_WARN(glGetError());

	for (int i = 0; i < numMaterials; i++) {
		if (resized_image_8)
			glTextureSubImage3D(m_Renderid, 0, 0, 0, i, 16, 16, 1, GL_RGB, GL_UNSIGNED_BYTE, resized_image_8);
		else
			glTextureSubImage3D(m_Renderid, 0, 0, 0, i, m_Width, m_Height, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel_data_8);
	}
}
