#include "hzpch.h"
#include "glad/glad.h"
#include "Atmosphere.h"

namespace Hazel {
	ref<Shader> Atmosphere::atmosphere_shader;
	void Atmosphere::RenderAtmosphere(Camera& camera,const float& atmosphere_radius)
	{
		//glDisable(GL_CULL_FACE);
		atmosphere_shader->Bind();
		atmosphere_shader->SetFloat3("camera_pos", camera.GetCameraPosition());
		atmosphere_shader->SetFloat3("view_dir", camera.GetViewDirection());

		glm::mat4 transform = glm::scale(glm::mat4(1.0), glm::vec3(atmosphere_radius));
		atmosphere_shader->SetMat4("u_ProjectionView", camera.GetProjectionView());
		atmosphere_shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
		atmosphere_shader->SetMat4("u_Model", transform);
		atmosphere_shader->SetMat4("u_View", camera.GetViewMatrix());
		atmosphere_shader->SetFloat("atmosphere_radius", atmosphere_radius);
		atmosphere_shader->SetInt("depthTex", DEPTH_SLOT);
		atmosphere_shader->SetInt("sceneTex", SCENE_TEXTURE_SLOT);
		atmosphere_shader->SetFloat3("sun_direction", Renderer3D::m_SunLightDir);
		atmosphere_shader->SetFloat("densityFalloff", 6.3);

		RenderQuad();
		//RenderCommand::DrawArrays(*Scene::Sphere->VertexArray, Scene::Sphere->Vertices.size());
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
	}
	void Atmosphere::InitilizeAtmosphere()
	{
		atmosphere_shader = Shader::Create("Assets/Shaders/Atmosphere_Shader.glsl");
	}
	void Atmosphere::RenderQuad()
	{
		//this function renders a quad infront of the camera
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);//disable depth testing

		//auto inv = glm::inverse(proj * glm::mat4(glm::mat3(view)));//get inverse of projection view to convert cannonical view to world space
		glm::vec4 data[] = {
		glm::vec4(-1,-1,0,1),glm::vec4(0,0,0,0),
		glm::vec4(1,-1,0,1),glm::vec4(1,0,0,0),
		glm::vec4(1,1,0,1),glm::vec4(1,1,0,0),
		glm::vec4(-1,1,0,1),glm::vec4(0,1,0,0)
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
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}