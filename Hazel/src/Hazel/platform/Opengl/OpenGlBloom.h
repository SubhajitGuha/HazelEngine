#pragma once
#include "Hazel/Renderer/Bloom.h"

namespace Hazel {
	class OpenGlBloom : public Bloom {
	public:
		OpenGlBloom();
		~OpenGlBloom();

		void InitBloom() override;
		void Update(TimeStep ts) override;
		void GetFinalImage(const unsigned int& img, const glm::vec2& dimension) override { m_Dimension = dimension; }
		void RenderBloomTexture() override { DownSample(); UpSample(); };

	protected:
		void DownSample() override;
		void UpSample() override;

	private:
		void RenderQuad();
		void ExtractImageBrightParts();
	private:
		unsigned int m_FBO;
		unsigned int m_InputImage,m_BrightImage;

		glm::vec2 m_Dimension = {0,0}, m_ScreenDimension;//note Screen dimension will not change as it is the dimension with which every thing is being rendered
		ref<Shader> m_DownSampleShader;
		ref<Shader> m_UpSampleShader,BloomToneMapShader,ExtractBrightParts;
	};
}