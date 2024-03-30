#pragma once
#include "Hazel/Renderer/Fog.h"

namespace Hazel
{
	class OpenGlFog :public Fog
	{
	public:
		OpenGlFog();
		OpenGlFog(float density, float fogStart, float fogEnd, float fogTop, float fogBottom, glm::vec2 ScreenSize);
		~OpenGlFog();

		void RenderFog(Camera& cam, glm::vec2 screenSize) override;
		void SetFogParameters(float density, float fogTop, float fogEnd,glm::vec3 fogColor) override { m_density = density, m_fogEnd = fogEnd, m_fogTop = fogTop, m_fogColor = fogColor; }

	private:
		void RenderQuad();
	private:
		ref<Shader> m_fogShader;
		float m_density, m_gradient, m_fogStart, m_fogEnd, m_fogTop, m_fogBottom;
		glm::vec3 m_fogColor = glm::vec3(1, 1, 1);
		uint32_t m_framebufferID, m_renderBufferID, m_textureID;
		glm::vec2 m_screenSize;
	};
}