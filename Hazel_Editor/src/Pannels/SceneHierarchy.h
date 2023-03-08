#pragma once
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Core.h"
#include "Hazel/Log.h"

namespace Hazel {
	class SceneHierarchyPannel {
	public:
		SceneHierarchyPannel();
		~SceneHierarchyPannel();
		void Context(const ref<Scene>& context);
		void OnImGuiRender();
	private:
		ref<Scene> m_Context;
		ref<Entity> m_selected_entity;

		//For properties pannel (camera ui)
		//glm::vec3 translation=glm::vec3(0);
	private:
		void DrawHierarchyNode(const Entity* ent);
		void DrawProperties();
		void DrawTransformUI();
		void DrawCameraUI();
		void DrawTagUI();
		void DrawSpriteRendererUI();
	};
}