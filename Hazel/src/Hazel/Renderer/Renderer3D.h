#pragma once
#include "Renderer2D.h"
#include "Hazel/LoadMesh.h"
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
		static void DrawMesh(LoadMesh& mesh);//take the mesh class reference
	private:
		friend class LoadMesh;
	};
}

