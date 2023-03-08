#pragma once
#include "Hazel.h"

namespace Hazel {
	class EditorCamera;
	class Shadows {
	public:
		Shadows();
		Shadows(float width, float height);
		virtual ~Shadows();
		virtual void RenderShadows(Scene& scene, const glm::vec3& LightPosition, EditorCamera& cam,ref<Shader> rendering_shader) = 0;
		virtual void SetShadowMapResolution(const float& width, float height) = 0;
		virtual unsigned int GetDepth_ID() = 0;
		static ref<Shadows> Create(float width, float height);
		static ref<Shadows> Create();//creates a texture map of 2048x2048 resolution

	};
}