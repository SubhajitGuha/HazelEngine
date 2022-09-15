#include "Sandbox2dApp.h"

SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1920.0 / 1080.0)
{
	tex2 = Texture2D::Create("Assets/Textures/Test.png");
	texture = Texture2D::Create("Assets/Textures/rickshaw.png");
	Renderer2D::Init();
}

void SandBox2dApp::OnAttach()
{

}

void SandBox2dApp::OnDetach()
{
}

void SandBox2dApp::OnUpdate(float deltatime)
{
	RenderCommand::ClearColor(glm::vec4(0.4, 0.4, 0.4, 1.0));
	RenderCommand::Clear();

	m_camera.OnUpdate(deltatime);

	if (Input::IsKeyPressed(HZ_KEY_UP))
		position.y += ObjSpeed * deltatime;
	if (Input::IsKeyPressed(HZ_KEY_DOWN))
		position.y -= ObjSpeed * deltatime;
	if (Input::IsKeyPressed(HZ_KEY_LEFT))
		position.x -= ObjSpeed * deltatime;
	if (Input::IsKeyPressed(HZ_KEY_RIGHT))
		position.x += ObjSpeed * deltatime;
	if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_4))
		scale += 0.3;
	if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_5))
		scale -= 0.3;

	glm::mat4 ModelTransform2 = glm::translate(glm::mat4(1), position) * glm::scale(glm::mat4(1), glm::vec3(scale));
	/*
	Set the model transform for now there is no scale or rotation
	just multiply the scale and rotation matrix with position to get the full transform
	*/
	Renderer2D::BeginScene(m_camera.GetCamera());
	Renderer2D::DrawQuad({ 0.5,0.8,-0.1 }, { 1,1,0 }, Color1);
	Renderer2D::DrawQuad(position, glm::vec3(scale), tex2);
	Renderer2D::DrawQuad({ 0.5,-0.1,0.10 }, { 1,1,0 }, texture);
	Renderer2D::EndScene();
}

void SandBox2dApp::OnImGuiRender()
{
	ImGui::Begin("Color Picker");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();
}

void SandBox2dApp::OnEvent(Event& e)
{
	m_camera.OnEvent(e);
}
