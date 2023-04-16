#pragma once
#include "Hazel/Core.h"
#include "Hazel.h"
#include "Hazel//Renderer/CubeMapReflection.h"
#include "Hazel/Renderer/Shadows.h"

namespace Hazel {
	class EditorCamera;
	class LoadMesh;
	class Renderer3D
	{
	public:
		static void Init();
		static void BeginScene(OrthographicCamera& camera);
		static void BeginScene(Camera& camera);
		static void BeginScene(EditorCamera&);
		static void BeginSceneFoliage(EditorCamera&);

		static void EndScene();
	public:
		static void SetSunLightDirection(const glm::vec3& dir);
		static void SetPointLightPosition(const std::vector<PointLight*>& Lights);
		static void DrawMesh(LoadMesh& mesh, const glm::vec3& Position, const glm::vec3& Scale = {1,1,1}, const glm::vec3& rotation = { 0,0,0 }, const glm::vec4& color = { 1,1,1,1 });//take the mesh class reference
		static void DrawMesh(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color = {1,1,1,1} ,const float& material_Roughness=1.0f,const float& material_metallic = 0.0f );//take the mesh class reference
		static void DrawFoliage(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color = { 1,1,1,1 }, const float& material_Roughness = 1.0f, const float& material_metallic = 0.0f);//take the mesh class reference
		static void SetUpCubeMapReflections(Scene& scene);
		static void RenderShadows(Scene& scene, EditorCamera& camera);
		static void AmbiantOcclusion(Scene& scene, EditorCamera& camera);

		static unsigned int depth_id;
	private:
		static glm::vec3 m_SunLightDir;
		friend class LoadMesh;
	};
}