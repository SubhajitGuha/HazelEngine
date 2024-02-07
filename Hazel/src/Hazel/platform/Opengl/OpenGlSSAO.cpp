#include "hzpch.h"
#include "OpenGlSSAO.h"
#include "glad/glad.h"
#include "Hazel/Renderer/Terrain.h"

namespace Hazel {
	OpenGlSSAO::OpenGlSSAO(int width, int height)
	{
		SSAOShader = Shader::Create("Assets/Shaders/SSAOShader.glsl");
		GbufferPosition = Shader::Create("Assets/Shaders/gBuffersShader.glsl");
		GbufferPosition_Terrain = Shader::Create("Assets/Shaders/gBufferShader_Terrain.glsl");
		GbufferPositionInstanced = Shader::Create("Assets/Shaders/gBufferShaderInstanced.glsl");
		SSAOShader_Terrain = Shader::Create("Assets/Shaders/SSAOShader_Terrain.glsl");
		SSAOblurShader = Shader::Create("Assets/Shaders/SSAOblurShader.glsl");
		SSAOShader_Instanced = Shader::Create("Assets/Shaders/SSAOShader_Instanced.glsl");

		CreateSSAOTexture(width,height);
		Init();
	}
	OpenGlSSAO::~OpenGlSSAO()
	{
	}
	void OpenGlSSAO::Init()
	{
	}

	void OpenGlSSAO::CaptureScene(Scene& scene , Camera& cam)
	{
		auto viewport_size = RenderCommand::GetViewportSize();

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
		}


		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOtexture_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("SSAO Framebuffer compleate -_- ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		SSAOShader->Bind();
		SSAOShader->SetFloat("ScreenWidth", viewport_size.x);
		SSAOShader->SetFloat("ScreenHeight", viewport_size.y);
		SSAOShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		SSAOShader->SetMat4("u_View", cam.GetViewMatrix());
		SSAOShader->SetFloat3Array("Samples", &samples[0].x, RANDOM_SAMPLES_SIZE);
		SSAOShader->SetMat4("u_projection", cam.GetProjectionMatrix());
		SSAOShader->SetInt("noisetex", NOISE_SLOT);
		SSAOShader->SetInt("gPosition", G_POSITION_TEXTURE_SLOT);
		SSAOShader->SetInt("gNormal", G_NORMAL_TEXTURE_SLOT);
		//SSAOShader->SetInt("alpha_texture", G_ROUGHNESS_METALLIC_TEXTURE_SLOT);//for foliage if the isFoliage flag is set to 1 then render the foliage with the help of opacity texture
		SSAOShader->SetFloat3("u_CamPos", cam.GetCameraPosition());
		RenderQuad();

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SSAOframebuffer_id);
		glViewport(0, 0, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, SSAOdepth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOblur_id, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("SSAO blur Framebuffer compleate ^_^ ");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		SSAOblurShader->Bind();
		SSAOblurShader->SetInt("SSAOtex", SSAO_SLOT);
		SSAOblurShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		SSAOblurShader->SetInt("alpha_texture", ROUGHNESS_SLOT);//for foliage if the isFoliage flag is set to 1 then render the foliage with the help of opacity texture
		RenderQuad();

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, viewport_size.x, viewport_size.y);
	}
	void OpenGlSSAO::CreateSSAOTexture(int width, int height)
	{
		m_width = width;
		m_height = height;
		auto size = RenderCommand::GetViewportSize();
		//create framebuffer and texture to store Ambiant occlusion texture
		glCreateFramebuffers(1, &SSAOframebuffer_id);

		glCreateTextures(GL_TEXTURE_2D, 1, &SSAOtexture_id);
		glBindTexture(GL_TEXTURE_2D, SSAOtexture_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

		//glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		glCreateTextures(GL_TEXTURE_2D, 1, &SSAOblur_id);
		glBindTexture(GL_TEXTURE_2D, SSAOblur_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glGenRenderbuffers(1, &SSAOdepth_id);
		glBindRenderbuffer(GL_RENDERBUFFER, SSAOdepth_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		std::uniform_real_distribution<float> RandomFloats(0.0, 1.0);//generate random floats between [0.0,1.0)
		std::default_random_engine generator; // random number generator

		std::vector<glm::vec3> noisetexture;
		for (int i = 0; i < 16; i++)
		{
			noisetexture.push_back(glm::vec3(RandomFloats(generator) * 2.0 - 1.0, RandomFloats(generator) * 2.0 - 1.0, 0.0f));
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
		
	}

	void OpenGlSSAO::RenderQuad()
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
		bl->push("coordinate", DataType::Float4);

		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		RenderCommand::DrawIndex(*vao);

		glDepthMask(GL_TRUE);//again enable depth testing
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	void OpenGlSSAO::RenderTerrain(Scene& scene, ref<Shader>& current_shader1, ref<Shader>& current_shader2)
	{			
		GbufferPositionInstanced->Bind();
		//Pass a alpha texture in the fragment shader to remove the depth values from the pixels that masked by alpha texture
		GbufferPositionInstanced->SetInt("u_Alpha", ROUGHNESS_SLOT);//'2' is the slot for roughness map (alpha, roughness , AO in RGB) I have explicitely defined it for now
		GbufferPositionInstanced->SetMat4("u_Model", Terrain::m_terrainModelMat);
		GbufferPositionInstanced->SetMat4("u_View", scene.GetCamera()->GetViewMatrix());
		GbufferPositionInstanced->SetMat4("u_Projection", scene.GetCamera()->GetProjectionMatrix());
		GbufferPositionInstanced->SetInt("Noise", PERLIN_NOISE_TEXTURE_SLOT);
		GbufferPositionInstanced->SetFloat("u_Time", Terrain::time);
		glCullFace(GL_BACK);
	}
}