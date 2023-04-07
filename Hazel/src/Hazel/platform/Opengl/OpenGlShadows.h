#pragma once
#include "Hazel.h"
#include "Hazel/Renderer/Shadows.h"
#define MAX_CASCADES 4
namespace Hazel{
	class OpenGlShadows:public Shadows
	{
	public:
		OpenGlShadows();
		OpenGlShadows(const float& width, const float& height);
		~OpenGlShadows();
		void RenderShadows(Scene& scene, const glm::vec3& LightPosition, EditorCamera& cam) override;
		void SetShadowMapResolution(const float& width, float height) override;
		void PassShadowUniforms(EditorCamera& cam, ref<Shader> rendering_shader) override;
		virtual unsigned int GetDepth_ID() override { return depth_id[0]; }
		void CreateShdowMap();
		
	private:
		void PrepareShadowProjectionMatrix(EditorCamera& camera,const glm::vec3& LightPosition);

	private:
		unsigned int depth_id[MAX_CASCADES],framebuffer_id;//max 4 cascades
		float m_width, m_height;
		ref<Shader> shadow_shader;
		LoadMesh* m_LoadMesh, *Cube, *Plane, *plant;//these needs to be changed
		std::vector<glm::mat4> m_ShadowProjection;
		std::vector<float> Ranges;
		glm::mat4 m_Camera_Projection;
		float m_ShadowAspectRatio = 1.0f;
		glm::mat4 LightView[MAX_CASCADES] = { glm::mat4(1.0f) };
		
		std::vector<int> n_cascades = { 0,1,2,3 };
	};
}