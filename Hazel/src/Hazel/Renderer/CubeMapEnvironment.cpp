#include "hzpch.h"
#include "CubeMapEnvironment.h"
#include "Hazel/Renderer/Camareas/EditorCamera.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "stb_image_resize.h"

namespace Hazel {
	ref<Shader> Cube_Shader;
	ref<Shader> irradiance_shader = nullptr;
	unsigned int CubeMapEnvironment::irradiance_map_id = 0, CubeMapEnvironment::framebuffer_id = 0;
	void CubeMapEnvironment::Init()
	{
		//irradiance_shader = Shader::Create("Assets/Shaders/irradianceCubeMapShader.glsl");
		Cube_Shader = (Shader::Create("Assets/Shaders/CubeMapShader.glsl"));

		unsigned int tex_id;
		int width = 1920, height = 1080, channels;
		float* cube_map_data = nullptr, *resized_image=nullptr;

		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

		std::string filename = "Assets/Textures/Outdoor-Cube-Map/";

		for (int i = 0; i < 6; i++)//iterate over 6 images each representing the side of a cube
		{
			
			stbi_set_flip_vertically_on_load(1);
			cube_map_data = stbi_loadf((filename + std::to_string(i) + ".png").c_str(), &width, &height, &channels, 0);
			if (!cube_map_data) {
				HAZEL_CORE_ERROR("Cube Map NOT LOADED !!");
			}
			else {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, cube_map_data);
				
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				stbi_image_free(cube_map_data);
			}

		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
		glBindTextureUnit(ENV_SLOT, tex_id);//use 18th slot ("explicitly binded")
		Cube_Shader->SetInt("env", ENV_SLOT);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void CubeMapEnvironment::RenderCubeMap(const glm::mat4& view, const glm::mat4& proj)
	{
		Cube_Shader->Bind();
		RenderQuad(view, proj);
	}
	void CubeMapEnvironment::ConstructIrradianceMap()
	{
		int width = 1080, height = 1080;
		irradiance_shader->Bind();
		irradiance_shader->SetInt("env", ENV_SLOT);

		glGenFramebuffers(1, &framebuffer_id);
		//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

		glGenTextures(1, &irradiance_map_id);
		glBindBuffer(GL_TEXTURE_CUBE_MAP, irradiance_map_id);

		for (int i = 0; i < 6; i++)//iterate over 6 images each representing the side of a cube
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < 6; i++)
		{
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map_id, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_ERROR("FrameBuffer compleate!!");
		}
		
		GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_buffers);//Tell the fragment shader to output the color in the RenderTarget GL_COLOR_ATTACHMENT0

		auto size = RenderCommand::GetViewportSize();

		EditorCamera cam = EditorCamera();
		cam.SetPerspctive(90.00f, 0.01, 1000);
		cam.SetUPVector({ 0,-1,0 });
		cam.SetCameraPosition({ 0,-1,0 });
		float pitch=0, yaw = 0;
		std::vector<glm::vec3> dir = { {1,0,0},{-1,0,0},{0,-1,0},{0,1,0},{0,0,1},{0,0,-1} };

		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
		glViewport(0, 0, width, width);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear the buffers each time
		for (int i = 0; i < 6; i++)
		{
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map_id, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_ERROR("FrameBuffer compleate!!");
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear the buffers each time
			HAZEL_CORE_ERROR(glGetError());

			SwitchToFace(i,pitch,yaw);//rotate the camera
			cam.RotateCamera(yaw, pitch);
			irradiance_shader->Bind();
			RenderQuad(cam.GetViewMatrix(), cam.GetProjectionMatrix());
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, 1920, 1080);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear the buffers each time
		glBindTextureUnit(11, irradiance_map_id);
	}
	void CubeMapEnvironment::SwitchToFace(int n, float& pitch, float& yaw)
	{
		switch (n)
		{
			//pitch , yaw
		case 0:
			pitch = 0;
			yaw = -90;
			break;
		case 1:
			pitch = 0;
			yaw = 90;
			break;
		case 2:
			pitch = 90.0f;
			yaw = 180;
			break;
		case 3:
			pitch = -90;
			yaw = 180;
			break;
		case 4:
			pitch = 0;
			yaw = 0;
			break;
		case 5:
			pitch = 0;
			yaw = 180;
			break;
		}
	}
	void CubeMapEnvironment::RenderQuad(const glm::mat4& view, const glm::mat4& proj)
	{
		//this function renders a quad infront of the camera
		glDepthMask(GL_FALSE);//disable depth testing

		auto inv = glm::inverse(proj * glm::mat4(glm::mat3(view)));//get inverse of projection view to convert cannonical view to world space
		glm::vec4 data[] = {
		glm::vec4(-1,1,0,1),inv * glm::vec4(-1,1,0,1),
		glm::vec4(1,1,0,1),	inv * glm::vec4(1,1,0,1),
		glm::vec4(1,-1,0,1),inv * glm::vec4(1,-1,0,1),
		glm::vec4(-1,-1,0,1),inv * glm::vec4(-1,-1,0,1),
		};

		ref<VertexArray> vao = VertexArray::Create();
		ref<VertexBuffer> vb = VertexBuffer::Create(&data[0].x, sizeof(data));
		unsigned int i_data[] = { 0,1,2,0,2,3 };
		ref<IndexBuffer> ib = IndexBuffer::Create(i_data, sizeof(i_data));

		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout

		bl->push("position", DataType::Float4);
		bl->push("direction", DataType::Float4);

		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		RenderCommand::DrawIndex(*vao);

		glDepthMask(GL_TRUE);//again enable depth testing
	}
}