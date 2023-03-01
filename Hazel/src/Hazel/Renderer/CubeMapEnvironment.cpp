#include "hzpch.h"
#include "CubeMapEnvironment.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "stb_image_resize.h"

namespace Hazel {
	ref<Shader> Cube_Shader;
	void CubeMapEnvironment::Init()
	{
		Cube_Shader = (Shader::Create("Assets/Shaders/CubeMapShader.glsl"));

		unsigned int tex_id;
		int width = 2048, height = 2048, channels;
		unsigned char* cube_map_data = nullptr, *resized_image=nullptr;

		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

		std::string filename = "Assets/Textures/Yokohama2/";

		for (int i = 0; i < 6; i++)//iterate over 6 images each representing the side of a cube
		{
			stbi_set_flip_vertically_on_load(1);
			cube_map_data = stbi_load((filename + std::to_string(i) + ".jpg").c_str(), &width, &height, &channels, 0);
			if (!cube_map_data) {
				HAZEL_CORE_ERROR("Cube Map NOT LOADED !!");
			}
			else {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, cube_map_data);
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				stbi_image_free(cube_map_data);
			}

		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
		//glActiveTexture(GL_TEXTURE1);
		glBindTextureUnit(5, tex_id);
		Cube_Shader->SetInt("env", 5);

		//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	void CubeMapEnvironment::RenderCubeMap(const glm::mat4& view, const glm::mat4& proj)
	{
		
		glDepthMask(GL_FALSE);//disable depth testing

		auto inv = glm::inverse(proj * glm::mat4(glm::mat3(view)));//get inverse of projection view to convert cannonical view to world space
		glm::vec4 data[] = {
		glm::vec4(-1,1,0,1),inv * glm::vec4(-1,1,0,1),
		glm::vec4(1,1,0,1),	inv* glm::vec4(1,1,0,1),
		glm::vec4(1,-1,0,1),inv* glm::vec4(1,-1,0,1),
		glm::vec4(-1,-1,0,1),inv* glm::vec4(-1,-1,0,1) };

		ref<VertexArray> vao = VertexArray::Create();
		ref<VertexBuffer> vb = VertexBuffer::Create(&data[0].x, sizeof(data));
		unsigned int i_data[] = { 0,1,2,0,2,3 };
		ref<IndexBuffer> ib = IndexBuffer::Create(i_data, sizeof(i_data));

		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout

		bl->push("position", DataType::Float4);
		bl->push("direction", DataType::Float4);

		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		Cube_Shader->Bind();
		RenderCommand::DrawIndex(*vao);

		glDepthMask(GL_TRUE);//again enable depth testing
	}
}