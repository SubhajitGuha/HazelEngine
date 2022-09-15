#pragma once
#include "Hazel/Core.h"
#include "OrthographicCamera.h"
#include "glm/glm.hpp"
#include "Shader.h"
#include "Texture.h"
namespace Hazel {

	class Renderer2D {
	public:
		static void Init();
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
	public:
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col);
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, ref<Texture2D> col);
	};
}