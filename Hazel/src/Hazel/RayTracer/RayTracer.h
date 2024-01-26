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
		//outputTextureID is the texID that will be used to accumulate the progressive rendered image
		void draw(Camera& cam, uint32_t& outputTextureID);
		void RenderLowResImage(Camera& cam);
		void copyAccmulatedImage(uint32_t& ImageToCopy);
		void RenderScreenSizeQuad();
	public:
		int image_width, image_height;
		float viewport_width, viewport_height;
		ref<BVH> bvh;
		uint16_t samples;
		static uint32_t m_Sampled_TextureID; //textureIDs to store the accmulated image
		static bool isViewportFocused;
		static glm::vec3 m_LightPos;
		static float m_LightStrength;
		static bool EnableSky;
		static int numBounces;
		static int samplesPerPixel;

	private:
		uint32_t m_RT_TextureID, m_LowRes_TextureID; //textureIDs to store the rendered and low resolution image
		uint32_t m_fbo; //framebuffer object
		bool isMoved = false;
		uint32_t ssbo_linearBVHNodes = -1, ssbo_rtTriangles = -1, ssbo_triangleIndices = -1,
			ssbo_arrMaterials = -1;
		uint16_t m_Binding;
		int frame_num,sample_count;
		glm::uvec2 tile_size;
		glm::uvec2 tile_index;
		glm::mat4 old_view;
		float m_focalLength;
		ref<Shader> cs_RayTracingShader, RayTracing_CopyShader;
		std::chrono::steady_clock::time_point StartTime;
	};
}