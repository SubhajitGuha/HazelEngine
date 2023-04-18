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
		GbufferPosition->SetInt("alpha_texture", ROUGHNESS_SLOT);//for foliage if the isFoliage flag is set to 1 then render the foliage with the help of opacity texture

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, 512, 512);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);// LOL this is required for proper capture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GBufferPos_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("GBuffer Position Framebuffer compleate -_- ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		RenderScene(scene, GbufferPosition);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

		//Generate Random samples
		std::uniform_real_distribution<float> RandomFloats(0.0, 1.0);//generate random floats between [0.0,1.0)
		std::default_random_engine generator; // random number generator
		for (int i = 0; i < RANDOM_SAMPLES_SIZE; i++)
		{
			samples[i] = glm::vec3(
				RandomFloats(generator) * 2.0 - 1.0,
				RandomFloats(generator) * 2.0 - 1.0,
				RandomFloats(generator)
			);
			samples[i] = glm::normalize(samples[i]);
			samples[i] *= RandomFloats(generator);

			float scale = (float)i / RANDOM_SAMPLES_SIZE;
			float val = 0.1 * scale * scale + (1.0 - 0.1) * scale * scale;
			samples[i] *= val;
			//HAZEL_CORE_ERROR("random float {} {} {}",samples[i].x, samples[i].y, samples[i].z);
		}

		SSAOShader->Bind();
		SSAOShader->SetFloat("ScreenWidth", viewport_size.x);
		SSAOShader->SetFloat("ScreenHeight", viewport_size.y);
		SSAOShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		SSAOShader->SetMat4("u_View", cam.GetViewMatrix());
		SSAOShader->SetFloat3Array("Samples", &samples[0].x, RANDOM_SAMPLES_SIZE);
		SSAOShader->SetMat4("u_projection", cam.GetProjectionMatrix());
		SSAOShader->SetInt("noisetex", NOISE_SLOT);
		SSAOShader->SetInt("GPosition", DEPTH_SLOT);
		SSAOShader->SetInt("alpha_texture", ROUGHNESS_SLOT);//for foliage if the isFoliage flag is set to 1 then render the foliage with the help of opacity texture
		SSAOShader->SetFloat3("u_CamPos", cam.GetCameraPosition());

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOtexture_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("SSAO Framebuffer compleate -_- ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//HAZEL_CORE_WARN(glGetError());
		RenderScene(scene, SSAOShader);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

		SSAOblurShader->Bind();
		SSAOblurShader->SetInt("SSAOtex", SSAO_SLOT);
		SSAOblurShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		SSAOblurShader->SetInt("alpha_texture", ROUGHNESS_SLOT);//for foliage if the isFoliage flag is set to 1 then render the foliage with the help of opacity texture

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOblur_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("SSAO blur Framebuffer compleate ^_^ ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//HAZEL_CORE_WARN(glGetError());
		RenderScene(scene, SSAOblurShader);
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

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);

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

		glBindTextureUnit(NOISE_SLOT, noisetex_id);
		glBindTextureUnit(SSAO_BLUR_SLOT, SSAOblur_id);
		glBindTextureUnit(SSAO_SLOT, SSAOtexture_id);
		glBindTextureUnit(DEPTH_SLOT, GBufferPos_id);
	}
	void OpenGlSSAO::RenderScene(Scene& scene , ref<Shader>& current_shader)
	{
		scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
			{
				Entity Entity(&scene, m_entity);
				auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
				auto mesh = Entity.GetComponent<StaticMeshComponent>();
				current_shader->SetMat4("u_Model", transform);

				//for foliage handeling
				if (mesh.isFoliage == true)
					current_shader->SetInt("isFoliage", 1);
				else
					current_shader->SetInt("isFoliage", 0);

				if (Entity.HasComponent<SpriteRenderer>()) {
					auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
					Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
				}
				else
					Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
			});
	}
}