#pragma once
#include "Hazel/Core.h"
#include "Hazel.h"
#include "Hazel//Renderer/CubeMapReflection.h"

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
		static void EndScene();
	public:
		static void SetLightPosition(const glm::vec3& pos);
		static void DrawMesh(LoadMesh& mesh);//take the mesh class reference
		static void DrawMesh(LoadMesh& mesh, const glm::vec3& Position, const glm::vec3& Scale = {1,1,1}, const glm::vec3& rotation = { 0,0,0 }, const glm::vec4& color = { 1,1,1,1 });//take the mesh class reference
		static void DrawMesh(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color = {1,1,1,1});//take the mesh class reference
		static void SetUpCubeMapReflections(Scene& scene);
	private:
		friend class LoadMesh;
	};
}