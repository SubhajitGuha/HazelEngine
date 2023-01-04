#pragma once
#include "Hazel.h"
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
	void Bezier_Curve(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4);//this needs to be an entity so change later
	void Hermite_Curve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& v0, const glm::vec2& v1);
private:
	OrthographicCameraController m_camera;
	glm::vec4 Color1 = { 1,1,1,1 };
	std::unordered_map<char, ref<SubTexture2D>> asset_map;
	std::string level_map,tree_map;
	glm::vec3 position = { 0,0,0 };
	float ObjSpeed = 20;
	float scale = 1;
	glm::vec2 m_ViewportSize = { 1920,1080 };
	bool isWindowFocused = false;
	Entity* Square_entity;
	Entity* camera_entity;
	bool IsMainCamera = false, IsMainCamera2 = true;//Main camera selector in ImGui
	glm::vec2 P0 = { 0,0 }, P1 = { 3,3 }, P2 = { 5,-5 }, P3 = {8,-8},c2 = { 2,3 }, c1 = { 0.3,-2.2 };
	float factor = 0.1;
	


	ref<Scene> m_scene;
	ref<Shader> shader;
	ref<VertexArray> vao;
	ref <Texture2D> texture, tex2;
	ref<SubTexture2D> tree,land,mud,water;
	ref<FrameBuffer> m_FrameBuffer;
};