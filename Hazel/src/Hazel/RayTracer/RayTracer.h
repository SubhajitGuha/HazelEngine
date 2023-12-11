#pragma once
#include "Hazel.h"

namespace Hazel {
	class RayTracer
	{
	public:
		RayTracer() ;
		RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples=2);
		void Init();
		void RenderImage(Camera& cam);

	public:
		int image_width, image_height;
		float viewport_width, viewport_height;
		uint16_t samples;
		static uint32_t m_RT_TextureID;
	private:
		uint16_t m_Binding;
		float m_focalLength;
		ref<Shader> cs_RayTracingShader;
	};
}