#pragma once
#include "Hazel.h"
#include "Pannels/SceneHierarchy.h"
#include "Pannels/ContentBrowser.h"
#include "Pannels/MaterialEditor.h"
#include <unordered_map>
using namespace Hazel;

class  HazelEditor :public Layer {
public:
	 HazelEditor();
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltatime) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Event& e) override;

private:
	OrthographicCameraController m_camera;
	glm::vec4 Color1 = { 1,1,1,1 };
	std::unordered_map<char, ref<SubTexture2D>> asset_map;
	std::string level_map,tree_map;
	glm::vec3 position = { 0,0,0 };
	float ObjSpeed = 20;
	float scale = 1;
	glm::vec2 m_ViewportSize = { 1920,1080 }, m_RTViewportSize = {1920,1080};
	bool isWindowFocused = false;
	Entity* Square_entity,*Square2,*Square3;
	Entity* camera_entity;
	bool IsMainCamera = false, IsMainCamera2 = true;//Main camera selector in ImGui
	glm::vec2 P0 = { 0,0 }, P1 = { 3,3 }, P2 = { 5,-5 }, P3 = {8,-8},c2 = { 2,3 }, c1 = { 0.3,-2.2 };
	float factor = 0.1;
	
	glm::vec3 SunDirection = { 0,1,0.6 }, PointLightPos = {5,-5,3};

	SceneHierarchyPannel m_Pannel;
	ContentBrowser m_ContentBrowser;
	MaterialEditor m_MaterialEditor;
	ref<Scene> m_scene;
	ref<Shader> shader;
	ref<VertexArray> vao;
	ref <Texture2D> texture, tex2;
	ref<SubTexture2D> tree,land,mud,water;
	ref<FrameBuffer> m_FrameBuffer,m_FrameBuffer2, m_FrameBuffer3, m_FrameBuffer4;

	float frame_time = 0;//capture the delta time
	int numFrame =0;
};