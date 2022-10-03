#include "Sandbox2dApp.h"
//#include "Hazel/Profiling.h"

SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1920.0 / 1080.0)
{
	//HZ_PROFILE_SCOPE("SandBox2dApp::SandBox2dApp()");
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

void SandBox2dApp::OnUpdate(float deltatime )
{

	HZ_PROFILE_SCOPE("SandBox2dApp::OnUpdate");
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
	}
		/*
		Set the model transform. for now there is no scale or rotation
		just multiply the scale and rotation matrix with position to get the full transform
		*/
		
		HZ_PROFILE_SCOPE("RENDER");
		{
			Renderer2D::BeginScene(m_camera.GetCamera());
			int slot = 0;
			for (int i = 0; i < 90; i += 2) {
				for (int j = 0; j < 90; j += 2)
				{
					Renderer2D::DrawQuad({ i,j,0.0 },0.0, { 1.0,1.0,0.0 }, (slot % 2 == 0) ? tex2 : texture, (slot) % 2+1);//assign slots other than 0 as 0 is the default white texture
				}
				slot++;
			}
			Renderer2D::DrawQuad(position,19.f, glm::vec3(scale), Color1);
			Renderer2D::EndScene();
		}
}

void SandBox2dApp::OnImGuiRender()
{
	HZ_PROFILE_SCOPE("ImGUI RENDER");

	ImGui::Begin("Color Picker");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();

	//ImGui::Begin("Benchmark");
	//	
	//for (auto item : TimerVar)
	//{
	//	char name[50]="";
	//	strcpy(name, item.name);
	//	strcat(name, " %.3fms");
	//ImGui::Text(name,item.Time);
	//}
	//TimerVar.clear();
	//ImGui::End();

}

void SandBox2dApp::OnEvent(Event& e)
{
	m_camera.OnEvent(e);
}
