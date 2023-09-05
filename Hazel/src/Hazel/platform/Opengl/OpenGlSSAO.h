#pragma once
#include "Hazel.h"
#define RANDOM_SAMPLES_SIZE 64
namespace Hazel {
	class OpenGlSSAO
	{
	public:
		OpenGlSSAO();
		~OpenGlSSAO();
		void Init();
		inline void SetSSAO_TextureDimension(int width, int height) { m_width = width, m_height = height; }
		void CaptureScene(Scene& scene , Camera& cam);
		unsigned int GetSSAOid() { return SSAOblur_id; }
	private:
		void CreateSSAOTexture();
		void RenderScene(Scene& scene , ref<Shader>& current_shader);// This will be changed later
		void RenderTerrain(Scene& scene, ref<Shader>& current_shader1, ref<Shader>& current_shader2);// This will be changed later
		void RenderQuad(ref<Shader>& current_shader);
		int m_width=2048, m_height=2048;
		unsigned int SSAOframebuffer_id,SSAOtexture_id,GBufferPos_id , SSAOdepth_id , SSAOblur_id, depth_id;
		unsigned int noisetex_id;
		ref<Shader> SSAOShader,GbufferPosition, GbufferPosition_Terrain, GbufferPositionInstanced, SSAOblurShader;//temporary
		ref<Shader> SSAOShader_Terrain, SSAOShader_Instanced;
		ref<FrameBuffer> framebuffer;
		glm::vec3 samples[RANDOM_SAMPLES_SIZE];
		//Camera cam;
	};
}