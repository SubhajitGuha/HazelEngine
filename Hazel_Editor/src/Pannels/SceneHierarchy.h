#pragma once
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Core.h"
#include "Hazel/Log.h"

using namespace Hazel;
class Hazel::PointLight;
class SceneHierarchyPannel {
public:
	SceneHierarchyPannel();
	~SceneHierarchyPannel();
	void Context(const ref<Scene>& context);
	void OnImGuiRender();
public:
	static ref<Entity> m_selected_entity;
private:
	ref<Scene> m_Context;
	PointLight* m_selected_Light = nullptr;

	glm::vec3 PointLight_position = { 0,0,0 }, PointLight_color = {1,1,1};
	//For properties pannel (camera ui)
	//glm::vec3 translation=glm::vec3(0);
private:
	void DrawHierarchyNode(const Entity* ent);
	void DrawProperties();
	void DrawTransformUI();
	void DrawCameraUI();
	void DrawTagUI();
	void DrawSpriteRendererUI();
	void DrawStaticMeshComponentUI();
	void DrawPhysicsComponentUI();
	void DrawScriptComponentUI();
	bool bDrawScript_comp = false;
};
