#include "hzpch.h"
#include "glad/glad.h"
#include "Hazel/Physics/Physics3D.h"
#include "OpenGlDeferredRenderer.h"
#include "Hazel/Renderer/Terrain.h"
#include "Hazel/Renderer/Antialiasing.h"

namespace Hazel
{
	uint32_t OpenGlDeferredRenderer::m_framebufferID, OpenGlDeferredRenderer::m_RenderBufferID,
		OpenGlDeferredRenderer::m_NormalBufferID, OpenGlDeferredRenderer::m_AlbedoBufferID,
		OpenGlDeferredRenderer::m_RoughnessMetallicBufferID, OpenGlDeferredRenderer::m_VelocityBufferID;

	ref<Shader> OpenGlDeferredRenderer::m_ForwardPassShader;
	ref<Shader> OpenGlDeferredRenderer::m_DefferedPassShader;
	static int m_width, m_height;
	void OpenGlDeferredRenderer::Init(int width, int height)
	{
		m_width = width;
		m_height = height;

		m_DefferedPassShader = Shader::Create("Assets/Shaders/DeferredPassShader.glsl");
		m_ForwardPassShader = Shader::Create("Assets/Shaders/ForwardPassShader.glsl");

		glCreateFramebuffers(1, &m_framebufferID);		
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);		

		glGenTextures(1, &m_NormalBufferID);
		glBindTexture(GL_TEXTURE_2D, m_NormalBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_NormalBufferID, 0);

		glGenTextures(1, &m_VelocityBufferID);
		glBindTexture(GL_TEXTURE_2D, m_VelocityBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_VelocityBufferID, 0);

		glGenTextures(1, &m_AlbedoBufferID);
		glBindTexture(GL_TEXTURE_2D, m_AlbedoBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_AlbedoBufferID, 0);

		glGenTextures(1, &m_RoughnessMetallicBufferID);
		glBindTexture(GL_TEXTURE_2D, m_RoughnessMetallicBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_RoughnessMetallicBufferID, 0);


		GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, buffers);

		//capture the depth of the scene on to a texture this will later be used for render the sky
		glGenTextures(1, &m_RenderBufferID);
		glBindTexture(GL_TEXTURE_2D, m_RenderBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_RenderBufferID, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_TRACE("G-Buffer Framebuffer compleate -_- ");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTextureUnit(G_NORMAL_TEXTURE_SLOT, m_NormalBufferID);
		glBindTextureUnit(G_VELOCITY_BUFFER_SLOT, m_VelocityBufferID);
		glBindTextureUnit(G_COLOR_TEXTURE_SLOT, m_AlbedoBufferID);
		glBindTextureUnit(G_ROUGHNESS_METALLIC_TEXTURE_SLOT, m_RoughnessMetallicBufferID);
		glBindTextureUnit(SCENE_DEPTH_SLOT, m_RenderBufferID);
	}
	
	void OpenGlDeferredRenderer::CreateBuffers(Scene* scene)
	{
		auto viewport_size = RenderCommand::GetViewportSize();

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		glViewport(0, 0, m_width, m_height); //set the viewport resolution same as gbuffer texture resolution
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene->m_Terrain->RenderTerrain(*scene->GetCamera());

		Renderer3D::BeginScene(*scene->GetCamera(), m_ForwardPassShader);
		scene->getRegistry().each([&](auto m_entity)
			{
				Entity Entity(scene, m_entity);
				if (Entity.GetComponent<StaticMeshComponent>().isFoliage == false)
				{					
					auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
					glm::vec4 color;

					auto mesh = Entity.GetComponent<StaticMeshComponent>();
					if (Entity.HasComponent<PhysicsComponent>())
					{
						auto physics_cmp = Entity.GetComponent<PhysicsComponent>();
						Physics3D::UpdateTransform(Entity.GetComponent<TransformComponent>(), physics_cmp);
					}

					if (Entity.HasComponent<SpriteRenderer>()) {
						auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
						//Renderer3D::SetTransperancy(SpriteRendererComponent.Transperancy);
						Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color * SpriteRendererComponent.Emission_Scale, SpriteRendererComponent.m_Roughness, SpriteRendererComponent.m_Metallic, m_ForwardPassShader);
					}
					else {
						//Renderer3D::SetTransperancy(1.0f);
						Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor,1.0f,0.0f,m_ForwardPassShader); // default color, roughness, metallic value
					}
				}
				Renderer3D::EndScene();
			});

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, viewport_size.x, viewport_size.y);

		m_DefferedPassShader->Bind();
		m_DefferedPassShader->SetFloat3("EyePosition", scene->GetCamera()->GetCameraPosition());

	}
	void OpenGlDeferredRenderer::DeferredPass()
	{
		glm::vec2 jitter = Antialiasing::GetJitterOffset();

		m_DefferedPassShader->Bind();
		m_DefferedPassShader->SetFloat("jitterX", jitter.x);
		m_DefferedPassShader->SetFloat("jitterY", jitter.y);
	
		m_DefferedPassShader->SetInt("depthBuffer", SCENE_DEPTH_SLOT);
		m_DefferedPassShader->SetInt("gNormal", G_NORMAL_TEXTURE_SLOT);
		m_DefferedPassShader->SetInt("gColor", G_COLOR_TEXTURE_SLOT);
		m_DefferedPassShader->SetInt("gRoughnessMetallic", G_ROUGHNESS_METALLIC_TEXTURE_SLOT);
		m_DefferedPassShader->SetInt("gVelocity", G_VELOCITY_BUFFER_SLOT);		
		m_DefferedPassShader->SetInt("History_Buffer", ORIGINAL_SCENE_TEXTURE_SLOT);
		//other uniforms are passed in the Renderer3D.cpp class

		//this function renders a quad infront of the camera
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);//disable writing to depth buffer

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

		glDepthMask(GL_TRUE);//again enable writing to depth buffer
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	uint32_t OpenGlDeferredRenderer::GetBuffers(int bufferInd)
	{
		if (bufferInd == 0)
			return m_NormalBufferID;
		if (bufferInd == 1)
			return m_VelocityBufferID;
		if (bufferInd == 2)
			return m_AlbedoBufferID;
		if (bufferInd == 3)
			return m_RoughnessMetallicBufferID;
	}
}