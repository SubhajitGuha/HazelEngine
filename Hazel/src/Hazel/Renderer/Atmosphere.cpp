#include "hzpch.h"
#include "glad/glad.h"
#include "Atmosphere.h"

namespace Hazel {
	ref<Shader> Atmosphere::atmosphere_shader;
	void Atmosphere::RenderAtmosphere(Camera& camera,const float& atmosphere_radius)
	{
		atmosphere_shader->Bind();
		atmosphere_shader->SetFloat("atmosphere_radius", 6471e3);
		atmosphere_shader->SetFloat("densityFalloff", 6);
		atmosphere_shader->SetFloat3("sun_direction", Renderer3D::m_SunLightDir);
		atmosphere_shader->SetFloat("planetRadius", 6371e3);
		atmosphere_shader->SetFloat("ScatteringStrength", 1);
		atmosphere_shader->SetFloat3("RayOrigin", glm::vec3(0, 6372e3, 0));
		atmosphere_shader->SetFloat3("view_dir", camera.GetViewDirection());
		atmosphere_shader->SetInt("u_DepthTexture", SCENE_DEPTH_SLOT); //used as a mask to render only the areas which are valid

		glm::vec3 wavelength = glm::vec3(700, 530, 440);
		glm::vec3 ScatteringCoeff = glm::vec3(0.00000519673, 0.0000121427, 0.0000296453);//glm::vec3(pow(200 / wavelength.x, 4), pow(200 / wavelength.y, 4), pow(200 / wavelength.z, 4));
		atmosphere_shader->SetFloat3("ScatteringCoeff", ScatteringCoeff * 1.f);
		RenderQuad(camera.GetViewMatrix(), camera.GetProjectionMatrix());
	}
	void Atmosphere::InitilizeAtmosphere()
	{
		atmosphere_shader = Shader::Create("Assets/Shaders/Atmosphere_Shader.glsl");
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