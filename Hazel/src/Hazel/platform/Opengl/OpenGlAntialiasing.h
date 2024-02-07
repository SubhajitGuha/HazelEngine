#pragma once
#include "Hazel.h"
#include "Hazel/Core.h"
#include "Hazel/Renderer/Antialiasing.h"

namespace Hazel
{
	class OpenGlAntialiasing:public Antialiasing
	{
	public:
		OpenGlAntialiasing(int width, int height);
		~OpenGlAntialiasing();
		void Init(int width, int height);
		void Update() override;
		void RenderQuad();
	private:
		ref<Shader> TAA_Shader;
		uint32_t m_CurrentColorBufferID, m_HistoryColorBufferID;
		uint32_t m_fbo, m_fbo_history;
		int m_width, m_height;
		int numFrame = 0;
	};
}