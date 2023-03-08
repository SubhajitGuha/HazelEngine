#include "hzpch.h"
#include "OpenGlShadows.h"
#include "glad/glad.h"

namespace Hazel {
	OpenGlShadows::OpenGlShadows()
		:m_width(2048), m_height(2048)
	{
	}
	OpenGlShadows::OpenGlShadows(const float& width, const float& height)
		:m_width(width),m_height(height)
	{
		RenderCommand::Init();
		shadow_shader = Shader::Create("Assets/Shaders/ShadowShader.glsl");//texture shader
		m_LoadMesh = new LoadMesh("Assets/Meshes/Sphere.obj");//these should be the part of entity component system!!
		Cube = new LoadMesh("Assets/Meshes/Cube.obj");
		Plane = new LoadMesh("Assets/Meshes/Plane.obj");
		m_ShadowProjection = glm::perspective(glm::radians(90.f), 1.0f, 1.f, 1000.0f);
		CreateShdowMap();
	}
	OpenGlShadows::~OpenGlShadows()
	{
	}
	void OpenGlShadows::RenderShadows(Scene& scene,const glm::vec3& LightPosition,EditorCamera& camera,ref<Shader> rendering_shader)
	{
		GLint OFb;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &OFb);
		auto size = RenderCommand::GetViewportSize();
		glm::mat4 Light = glm::lookAt(LightPosition, glm::vec3(0, 0, 0) , { 0,1,0 });
		glm::mat4 LightProjection = m_ShadowProjection * Light;

		rendering_shader->SetMat4("MatrixShadow", LightProjection);
		shadow_shader->Bind();
		shadow_shader->SetMat4("LightProjection", LightProjection);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
		glViewport(0, 0, m_width, m_height);
		glClear(GL_DEPTH_BUFFER_BIT);

		scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
			{
				//auto entt = item.second->GetEntity();//get the original entity (i.e. entt::entity returns an unsigned int)
				Entity Entity(&scene, m_entity);
				auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
				glm::vec4 color;

				//if (camera.camera.bIsMainCamera) {
				if (Entity.HasComponent<SpriteRenderer>()) {
					auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
					Renderer3D::DrawMesh(*m_LoadMesh, transform, SpriteRendererComponent.Color);
				}
				else
					Renderer3D::DrawMesh(*Cube, transform, Entity.m_DefaultColor);
			});
		Renderer3D::DrawMesh(*Plane, { 0,0,0 }, { 100,100,100 }, { 0,0,0 });

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, size.x,size.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTextureUnit(7, depth_id);		
	}
	void OpenGlShadows::SetShadowMapResolution(const float& width, float height)
	{
		m_height = height;
		m_width = width;
	}
	void OpenGlShadows::CreateShdowMap()
	{
		glGenFramebuffers(1, &framebuffer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

		glGenTextures(1, &depth_id);
		glBindTexture(GL_TEXTURE_2D, depth_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");

		glBindTextureUnit(7, depth_id);
	}
}