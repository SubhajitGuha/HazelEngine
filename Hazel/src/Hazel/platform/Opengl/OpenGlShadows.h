#pragma once
#include "Hazel.h"
#include "Hazel/Renderer/Shadows.h"

namespace Hazel{
	class OpenGlShadows:public Shadows
	{
	public:
		OpenGlShadows();
		OpenGlShadows(const float& width, const float& height);
		~OpenGlShadows();
		void RenderShadows(Scene& scene, const glm::vec3& LightPosition, EditorCamera& cam, ref<Shader> rendering_shader) override;
		virtual void SetShadowMapResolution(const float& width, float height) override;
		virtual unsigned int GetDepth_ID() override { return depth_id; }
		void CreateShdowMap();
	private:
		unsigned int depth_id,framebuffer_id;
		float m_width, m_height;
		ref<Shader> shadow_shader;
		LoadMesh* m_LoadMesh, *Cube, *Plane;//these needs to be changed
		glm::mat4 m_ShadowProjection;
	};
}