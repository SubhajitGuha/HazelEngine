#pragma once
#include "Hazel.h"
#include "Hazel/Renderer/Material.h"
#include "BVH.h"

namespace Hazel {
	class RayTracer
	{
	public:
		RayTracer() ;
		RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples=2);
		void RenderImage(Camera& cam);
		void Resize(int width, int height);
		void UpdateScene();
	private:
		void Init(int width, int height);
	public:
		int image_width, image_height;
		float viewport_width, viewport_height;
		uint16_t samples;
		static uint32_t m_RT_TextureID;

		static glm::vec4 m_Color;
		//static glm::vec4 m_SphereEmissionCol;
		//static std::vector<float> m_SphereEmissionStrength;
		static float m_Roughness;
		static bool EnableSky;
		static int numBounces;
		static int samplesPerPixel;

		//static std::vector<float> m_SphereSpecStrength;

	private:
		uint32_t ssbo_linearBVHNodes = -1, ssbo_rtTriangles = -1, ssbo_triangleIndices = -1;
		uint16_t m_Binding;
		int frame_num;
		glm::mat4 old_view;
		float m_focalLength;
		ref<Shader> cs_RayTracingShader;
		ref<BVH> bvh;
		std::chrono::steady_clock::time_point StartTime;
	};
}