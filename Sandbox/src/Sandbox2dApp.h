#pragma once
#include "Hazel.h"
#include <unordered_map>

using namespace Hazel;

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

	ref<Scene> m_Scene;
	ref<Shader> shader;
	ref<VertexArray> vao;
	ref <Texture2D> texture, tex2;
	ref<SubTexture2D> tree,land,mud,water;
};