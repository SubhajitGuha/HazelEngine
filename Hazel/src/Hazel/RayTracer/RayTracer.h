#pragma once
#include "Hazel.h"

namespace Hazel {
	class RayTracer
	{
	public:
		RayTracer() ;
		RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples=2);
		void RenderImage(Camera& cam);
		void Resize(int width, int height);
	private:
		void Init(int width, int height);
	public:
		int image_width, image_height;
		float viewport_width, viewport_height;
		uint16_t samples;
		static uint32_t m_RT_TextureID;
	private:
		uint16_t m_Binding;
		float m_focalLength;
		ref<Shader> cs_RayTracingShader;
		std::chrono::steady_clock::time_point StartTime;
	};
}