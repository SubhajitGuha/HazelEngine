#pragma once
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/RenderCommand.h"

namespace Hazel {
	class CubeMapEnvironment {
	public:
		static void Init(const std::string& path);
		static void RenderCubeMap(glm::mat4& view, glm::mat4& proj, const glm::vec3& view_dir);
	private:
		static uint32_t irradiance_map_id, tex_id;
		static uint32_t framebuffer_id, framebuffer_id2;
		static uint32_t hdrMapID;
		static uint32_t renderBuffer_id;
		static uint32_t captureRes;
		static ref<Shader> Cube_Shader, irradiance_shader, equirectangularToCube_shader, prefilterShader, BRDFSumShader;
	private:
		static void SwitchToFace(int n, float& pitch, float& yaw);
		static void ConstructIrradianceMap(glm::mat4 proj);
		static void CreateSpecularMap(glm::mat4& proj,glm::mat4*);
		static void RenderQuad(const glm::mat4& view, const glm::mat4& proj);//used for cube-maps (direction is passed in 2nd slot)
		static void RenderQuad();//used for 2D-Textures (Texture-coordinate is passed in 2nd slot)
	};
}