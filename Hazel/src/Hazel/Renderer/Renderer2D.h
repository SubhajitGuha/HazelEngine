#pragma once
#include "Hazel/Core.h"
#include "OrthographicCamera.h"
#include "glm/glm.hpp"
#include "Shader.h"
#include "Texture.h"
#include "SubTexture.h"
namespace Hazel {

	class Renderer2D {
	public:
		static void Init();
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
	public:
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col, const float angle = 0.f);
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, ref<Texture2D> tex, unsigned int index = 1, const float angle = 0.f);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawSubTexturedQuad(const glm::vec3& pos, const glm::vec3& scale, ref<SubTexture2D> tex, unsigned int index = 2, const float angle = 0.f);
		//note texture coord defines the uv coordinate of the texture (in sprite sheet we need these texture coordinates to get individual asset images)
	};
}