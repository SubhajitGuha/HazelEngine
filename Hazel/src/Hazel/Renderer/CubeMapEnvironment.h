#pragma once
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/RenderCommand.h"

namespace Hazel {
	class CubeMapEnvironment {
	public:
		static void Init();
		static void RenderCubeMap(const glm::mat4& view, const glm::mat4& proj);
		static void ConstructIrradianceMap();
		static void RenderQuad(const glm::mat4& view, const glm::mat4& proj);
	private:
		static unsigned int irradiance_map_id;
		static unsigned int framebuffer_id;
	private:
		static void SwitchToFace(int n, float& pitch, float& yaw);
	};
}