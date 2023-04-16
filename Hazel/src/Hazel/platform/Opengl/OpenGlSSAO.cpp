#include "hzpch.h"
#include "OpenGlSSAO.h"
#include "glad/glad.h"

namespace Hazel {
	OpenGlSSAO::OpenGlSSAO()
		:m_height(1024),m_width(1024)
	{
		SSAOShader = Shader::Create("Assets/Shaders/SSAOShader.glsl");
		GbufferPosition = Shader::Create("Assets/Shaders/gBuffersShader.glsl");
		SSAOblurShader = Shader::Create("Assets/Shaders/SSAOblurShader.glsl");

		CreateSSAOTexture();
		Init();
	}
	OpenGlSSAO::~OpenGlSSAO()
	{
	}
	void OpenGlSSAO::Init()
	{
	}

	void OpenGlSSAO::CaptureScene(Scene& scene , EditorCamera& cam)
	{
		auto viewport_size = RenderCommand::GetViewportSize();

		GbufferPosition->Bind();
		GbufferPosition->SetMat4("u_ProjectionView", cam.GetProjectionView());
		GbufferPosition->SetMat4("u_View", cam.GetViewMatrix());

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);// LOL this is required for proper capture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GBufferPos_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("GBuffer Position Framebuffer compleate -_- ");
		HAZEL_CORE_WARN(glGetError());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
			{
				Entity Entity(&scene, m_entity);
				auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
				auto mesh = Entity.GetComponent<StaticMeshComponent>();
				if (Entity.HasComponent<SpriteRenderer>()) {
					auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
					Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
				}
				else
					Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
			});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

		//Generate Random samples
		//std::uniform_real_distribution<float> RandomFloats(0.0, 1.0);//generate random floats between [0.0,1.0)
		//std::default_random_engine generator; // random number generator

		SSAOShader->Bind();
		SSAOShader->SetFloat("ScreenWidth", m_width);
		SSAOShader->SetFloat("ScreenHeight", m_height);
		SSAOShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		SSAOShader->SetMat4("u_View", cam.GetViewMatrix());
		SSAOShader->SetFloat3Array("Samples", &samples[0].x, 128);
		SSAOShader->SetMat4("u_projection", cam.GetProjectionMatrix());
		SSAOShader->SetInt("noisetex", 20);
		SSAOShader->SetInt("GPosition", DEPTH_SLOT);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOtexture_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("SSAO Framebuffer compleate -_- ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//HAZEL_CORE_WARN(glGetError());
		scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
			{
				Entity Entity(&scene, m_entity);
				auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
				auto mesh = Entity.GetComponent<StaticMeshComponent>();
				if (Entity.HasComponent<SpriteRenderer>()) {
					auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
					Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
				}
				else
					Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
			});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

		SSAOblurShader->Bind();
		SSAOblurShader->SetInt("SSAOtex", SSAO_SLOT);
		SSAOblurShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOblur_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("SSAO blur Framebuffer compleate ^_^ ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//HAZEL_CORE_WARN(glGetError());
		scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
			{
				Entity Entity(&scene, m_entity);
				auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
				auto mesh = Entity.GetComponent<StaticMeshComponent>();
				if (Entity.HasComponent<SpriteRenderer>()) {
					auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
					Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
				}
				else
					Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
			});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

	}
	void OpenGlSSAO::CreateSSAOTexture()
	{
		auto size = RenderCommand::GetViewportSize();
		//create framebuffer and texture to store Ambiant occlusion texture
		glCreateFramebuffers(1, &SSAOframebuffer_id);

		glCreateTextures(GL_TEXTURE_2D,1, &SSAOtexture_id);
		glBindTexture(GL_TEXTURE_2D, SSAOtexture_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_FLOAT, nullptr);

		//glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		glCreateTextures(GL_TEXTURE_2D, 1, &SSAOblur_id);
		glBindTexture(GL_TEXTURE_2D, SSAOblur_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glGenRenderbuffers(1, &SSAOdepth_id);
		glBindRenderbuffer(GL_RENDERBUFFER, SSAOdepth_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		glGenTextures(1, &GBufferPos_id);//GBuffer view space position texture
		glBindTexture(GL_TEXTURE_2D, GBufferPos_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		std::uniform_real_distribution<float> RandomFloats(0.0, 1.0);//generate random floats between [0.0,1.0)
		std::default_random_engine generator; // random number generator

		for (int i = 0; i < 128; i++)
		{
			samples[i] = glm::vec3(
				RandomFloats(generator) * 2.0 - 1.0,
				RandomFloats(generator) * 2.0 - 1.0,
				RandomFloats(generator)
			);
			samples[i] = glm::normalize(samples[i]);
			samples[i] *= RandomFloats(generator);

			float scale = (float)i / 64.0f;
			float val = 0.1 * scale * scale + (1.0 - 0.1) * scale * scale;
			samples[i] *= val;
			//HAZEL_CORE_ERROR("random float {} {} {}",samples[i].x, samples[i].y, samples[i].z);
		}

		std::vector<glm::vec3> noisetexture;
		for (int i = 0; i < 16; i++)
		{
			noisetexture.push_back(glm::vec3(RandomFloats(generator) * 2.0 - 1.0, RandomFloats(generator) * 2.0 - 1.0, 0.0));
		}
		glCreateTextures(GL_TEXTURE_2D, 1, &noisetex_id);
		glBindTexture(GL_TEXTURE_2D, noisetex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noisetexture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		HAZEL_CORE_WARN(glGetError());

		glBindTextureUnit(20, noisetex_id);
		glBindTextureUnit(SSAO_BLUR_SLOT, SSAOblur_id);
		glBindTextureUnit(SSAO_SLOT, SSAOtexture_id);
		glBindTextureUnit(DEPTH_SLOT, GBufferPos_id);
	}
	void OpenGlSSAO::RenderScene(Scene& scene, EditorCamera& editor_cam)
	{
		
	}
}