#pragma once
#include "Hazel/Renderer/Fog.h"

namespace Hazel
{
	class OpenGlFog :public Fog
	{
	public:
		OpenGlFog();
		OpenGlFog(float density, float gradient, float fogStart, float fogEnd, glm::vec2 ScreenSize);
		~OpenGlFog();

		void RenderFog(Camera& cam, glm::vec2 screenSize) override;

	private:
		void RenderQuad();
	private:
		ref<Shader> m_fogShader;
		float m_density, m_gradient, m_fogStart, m_fogEnd;
		uint32_t m_framebufferID, m_renderBufferID, m_textureID;
		glm::vec2 m_screenSize;
	};
}