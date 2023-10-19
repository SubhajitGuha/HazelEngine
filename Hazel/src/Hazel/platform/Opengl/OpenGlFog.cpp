#include "hzpch.h"
#include "OpenGlFog.h"
#include "glad/glad.h"

namespace Hazel
{
	OpenGlFog::OpenGlFog()
		:m_density(0.005),m_gradient(1.3),m_fogStart(60.0f),m_fogEnd(5000)
	{
		m_fogShader = Shader::Create("Assets/Shaders/pp_fogShader.glsl");
	}
	OpenGlFog::OpenGlFog(float density, float gradient, float fogStart, float fogEnd, glm::vec2 ScreenSize)
		: m_density(density), m_gradient(gradient), m_fogStart(fogStart), m_fogEnd(fogEnd), m_screenSize(ScreenSize)
	{
		m_fogShader = Shader::Create("Assets/Shaders/pp_fogShader.glsl");

		glGenFramebuffers(1, &m_framebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_screenSize.x, m_screenSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);
		GLenum buff[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buff);
	}
	OpenGlFog::~OpenGlFog()
	{
	}
	void OpenGlFog::RenderFog(Camera& cam, glm::vec2 screenSize)
	{
		m_fogShader->Bind();
		m_fogShader->SetInt("u_sceneDepth", SCENE_DEPTH_SLOT);
		m_fogShader->SetInt("u_sceneColor", ORIGINAL_SCENE_TEXTURE_SLOT);
		m_fogShader->SetFloat("u_nearPlane", cam.GetPerspectiveNear());
		m_fogShader->SetFloat("u_farPlane", cam.GetPerspectiveFar());
		m_fogShader->SetFloat("u_density", m_density);
		m_fogShader->SetFloat("u_gradient", m_gradient);

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
		glViewport(0, 0, m_screenSize.x, m_screenSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenSize.x, screenSize.y);

		glBindTextureUnit(ORIGINAL_SCENE_TEXTURE_SLOT, m_textureID);//update these slotes to capture the post-processing
		glBindTextureUnit(SCENE_TEXTURE_SLOT, m_textureID);

	}


	void OpenGlFog::RenderQuad()
	{
		//this function renders a quad infront of the camera
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);

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

		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}