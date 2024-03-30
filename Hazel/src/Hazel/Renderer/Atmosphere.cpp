#include "hzpch.h"
#include "glad/glad.h"
#include "Atmosphere.h"

namespace Hazel {
	ref<Shader> Atmosphere::atmosphere_shader;
	ref<Texture2DArray> Atmosphere::skyGradients;

	void Atmosphere::RenderAtmosphere(Camera& camera,const float& atmosphere_radius)
	{
		skyGradients->Bind(ALBEDO_SLOT);
		atmosphere_shader->Bind();
		atmosphere_shader->SetFloat3("sun_direction", Renderer3D::m_SunLightDir);
		atmosphere_shader->SetInt("Sky_Gradient", ALBEDO_SLOT);

		glm::vec3 wavelength = glm::vec3(700, 530, 440);
		glm::vec3 ScatteringCoeff = glm::vec3(0.00000519673, 0.0000121427, 0.0000296453);//glm::vec3(pow(200 / wavelength.x, 4), pow(200 / wavelength.y, 4), pow(200 / wavelength.z, 4));
		atmosphere_shader->SetFloat3("ScatteringCoeff", ScatteringCoeff * 1.f);
		RenderQuad(camera.GetViewMatrix(), camera.GetProjectionMatrix());
	}
	void Atmosphere::InitilizeAtmosphere()
	{
		atmosphere_shader = Shader::Create("Assets/Shaders/Atmosphere_Shader.glsl");

		std::vector<std::string> gradientTex_paths = {"Assets/Textures/Sky_Gradient_Textures/SunZenith_Gradient.png",
			"Assets/Textures/Sky_Gradient_Textures/ViewZenith_Gradient.png", "Assets/Textures/Sky_Gradient_Textures/SunView_Gradient.png"};
		skyGradients = Texture2DArray::Create(gradientTex_paths);
	}
	void Atmosphere::RenderQuad(const glm::mat4& view, const glm::mat4& proj)
	{
		//this function renders a quad infront of the camera
		glDepthMask(GL_FALSE);//disable writing to depth buffer

		glm::mat4 inv = glm::inverse(proj * glm::mat4(glm::mat3(view)));//get inverse of projection view to convert cannonical view to world space
		glm::vec4 data[] = {
		glm::vec4(-1,-1,0,1),inv * glm::vec4(-1,-1,0,1),
		glm::vec4(1,-1,0,1),inv * glm::vec4(1,-1,0,1),
		glm::vec4(1,1,0,1),	inv * glm::vec4(1,1,0,1),
		glm::vec4(-1,1,0,1),inv * glm::vec4(-1,1,0,1),
		};

		ref<VertexArray> vao = VertexArray::Create();
		ref<VertexBuffer> vb = VertexBuffer::Create(&data[0].x, sizeof(data));
		unsigned int i_data[] = { 0,1,2,0,2,3 };
		//unsigned int i_data[] = { 2,1,0,3,2,0};
		ref<IndexBuffer> ib = IndexBuffer::Create(i_data, sizeof(i_data));

		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout

		bl->push("position", DataType::Float4);
		bl->push("direction", DataType::Float4);

		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		RenderCommand::DrawIndex(*vao);

		glDepthMask(GL_TRUE);//again enable depth buffer write
	}
}