#pragma once
#include "Hazel.h"
#define RANDOM_SAMPLES_SIZE 128
namespace Hazel {
	class OpenGlSSAO
	{
	public:
		OpenGlSSAO();
		~OpenGlSSAO();
		void Init();
		inline void SetSSAO_TextureDimension(int width, int height) { m_width = width, m_height = height; }
		void CaptureScene(Scene& scene , EditorCamera& cam);
		unsigned int GetSSAOid() { return SSAOblur_id; }
	private:
		void CreateSSAOTexture();
		void RenderScene(Scene& scene);// This will be changed later
		int m_width=2048, m_height=2048;
		unsigned int SSAOframebuffer_id,SSAOtexture_id,GBufferPos_id , SSAOdepth_id , SSAOblur_id;
		unsigned int noisetex_id;
		ref<Shader> SSAOShader,GbufferPosition, SSAOblurShader;//temporary
		ref<FrameBuffer> framebuffer;
		glm::vec3 samples[RANDOM_SAMPLES_SIZE];
		//EditorCamera cam;
	};
}