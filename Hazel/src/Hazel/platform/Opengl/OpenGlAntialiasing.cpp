#include "hzpch.h"
#include "OpenGlAntialiasing.h"
#include "glad/glad.h"

namespace Hazel
{
	OpenGlAntialiasing::OpenGlAntialiasing(int width, int height)		
	{
		Init(width,height);
	}
	OpenGlAntialiasing::~OpenGlAntialiasing()
	{
	}
	void OpenGlAntialiasing::Init(int width, int height)
	{
		m_width = width; m_height = height;
		TAA_Shader = Shader::Create("assets/Shaders/TAA_Shader.glsl");

		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		glGenTextures(1, &m_CurrentColorBufferID);
		glBindTexture(GL_TEXTURE_2D, m_CurrentColorBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_CurrentColorBufferID, 0);

		GLenum buff[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buff);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void OpenGlAntialiasing::Update()
	{
		numFrame++;
		glm::vec2 screenSize = RenderCommand::GetViewportSize();
		if (m_width != screenSize.x || m_height != screenSize.y)
		{
			Init(screenSize.x, screenSize.y);
		}
		TAA_Shader->Bind();
		TAA_Shader->SetInt("History_Buffer", HISTORY_TEXTURE_SLOT);		
		TAA_Shader->SetInt("Current_Buffer", ORIGINAL_SCENE_TEXTURE_SLOT);
		TAA_Shader->SetInt("Depth_Buffer", SCENE_DEPTH_SLOT);
		TAA_Shader->SetInt("gVelocity", G_VELOCITY_BUFFER_SLOT);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);		
		RenderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);		

		glBindTextureUnit(HISTORY_TEXTURE_SLOT, m_CurrentColorBufferID);
		glBindTextureUnit(ORIGINAL_SCENE_TEXTURE_SLOT, m_CurrentColorBufferID);//update these slotes to capture the post-processing
		glBindTextureUnit(SCENE_TEXTURE_SLOT, m_CurrentColorBufferID);
	}
	void OpenGlAntialiasing::RenderQuad()
	{
		//this function renders a quad infront of the camera
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);//disable depth testing

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