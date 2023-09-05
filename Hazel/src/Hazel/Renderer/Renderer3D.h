#pragma once
#include "Hazel/Core.h"
#include "Hazel.h"
#include "Hazel//Renderer/CubeMapReflection.h"
#include "Hazel/Renderer/Shadows.h"

namespace Hazel {
	class Camera;
	class LoadMesh;
	class Renderer3D
	{
	public:
		static void Init();
		static void BeginScene(OrthographicCamera& camera);
		//static void BeginScene(Camera& camera);
		static void BeginScene(Camera&);
		static void BeginSceneFoliage(Camera&);

		static void EndScene();
	public:
		static void SetSunLightDirection(const glm::vec3& dir);
		static void SetSunLightColorAndIntensity(const glm::vec3& color, float Intensity);
		static void SetPointLightPosition(const std::vector<PointLight*>& Lights);
		static void DrawMesh(LoadMesh& mesh, const glm::vec3& Position, const glm::vec3& Scale = {1,1,1}, const glm::vec3& rotation = { 0,0,0 }, const glm::vec4& color = { 1,1,1,1 });//take the mesh class reference
		static void DrawMesh(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color = {1,1,1,1} ,const float& material_Roughness=1.0f,const float& material_metallic = 0.0f );//take the mesh class reference
		static void DrawFoliage(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color = { 1,1,1,1 }, const float& material_Roughness = 1.0f, const float& material_metallic = 0.0f);//take the mesh class reference
		static void DrawFoliageInstanced(LoadMesh& mesh, glm::mat4& transform,size_t instance_count, const glm::vec4& color = { 1,1,1,1 }, float TimeElapsed=0, const float& material_Roughness = 1.0f, const float& material_metallic = 0.0f);//take the mesh class reference
		static void InstancedFoliageData(LoadMesh& mesh, const std::vector<glm::mat4>& Instanced_ModelMatrix);
		static void SetUpCubeMapReflections(Scene& scene);
		static void RenderShadows(Scene& scene, Camera& camera);
		static void AmbiantOcclusion(Scene& scene, Camera& camera);
		static void SetTransperancy(float val);
	public:
		static unsigned int depth_id[4];
		static int index;
		static unsigned int ssao_id;
		static glm::vec3 m_SunLightDir;
		static glm::vec3 m_SunColor;
		static float m_SunIntensity;
	private:
		friend class LoadMesh;
	};
}