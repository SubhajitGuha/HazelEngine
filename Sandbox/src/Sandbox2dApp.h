#pragma once
#include "Hazel.h"
#include <unordered_map>
using namespace Hazel;

//Trading software build client side
class SandBox2dApp :public Layer {
public:
	SandBox2dApp();
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
	glm::vec3 position = { 0,0,0.2 };
	float ObjSpeed = 20;
	float scale = 1;
	glm::vec2 m_ViewportSize = { 1920,1080 };
	Entity* SquareEntt;
	Entity* CameraEntt;
	glm::vec2 P0 = { 0,0 }, P1 = { 3,3 }, P2 = { 5,-5 }, P3 = { 8,-8 }, c2 = { 2,3 }, c1 = { 0.3,-2.2 };
	//these coordinates are scaled to 100 (i.e) coord {3,3} is actually {300,300}
	float factor = 0.1;
	std::vector<glm::vec2> m_Points;

	ref<Scene> m_Scene;
	ref<Shader> shader;
	ref<VertexArray> vao;
	ref <Texture2D> texture, tex2;
	ref<SubTexture2D> tree,land,mud,water;
	ref<FrameBuffer> m_Framebuffer;
};