#pragma once
#include "Hazel.h"

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
	bool m_Running = true;
	std::shared_ptr<Shader> shader;
	OrthographicCameraController m_camera;
	ref<VertexArray> vao;
	glm::vec4 Color1 = { 1,1,1,1 };
	ref <Texture2D> texture, tex2;
	glm::vec3 position = { 0,0,0 };
	float ObjSpeed = 20;
	float scale = 1;
};