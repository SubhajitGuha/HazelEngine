#include "Sandbox2dApp.h"
//#include "Hazel/Profiling.h"

SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1920.0 / 1080.0)
{
	//HZ_PROFILE_SCOPE("SandBox2dApp::SandBox2dApp()");

	level_map =
		"llllllllllllllllllllllllllllll"
		"lllmlllllllwwwwwllllllllllllll"
		"lllmmllllwwwwwwwwwwlllllllllll"
		"llllllwwwwwwwwwwwwwwwwwlllllll"
		"llllwwwwwwwwwwwwwwwwwwmwwlllll"
		"lllwwwwwwwwmmmmwwwwwwwwmwwwlll"
		"llllllwmwwwwwwwwwwwwwwwlllllll"
		"llllwwwwwwwwwwwwwwwwllllllllll"
		"lllllllllwwwwwwwwwwwllllllllll"
		"llllllllmmmwwwwwllllllllllllll"
		"llllllllllllllllllllllllllllll";

	tree_map = 
		"ttttt  ttttt  tttt  ttttt  ttt"
		"ttt t     t     tttttttttttttt"
		"ttt  tttt          ttttttttttt"
		"tt  tt                 ttt   t"
		"tttt                     ttttt"
		"ttt                        ttt"
		"tttttt                 ttt  tt"
		"tttt                tttt    tt"
		"     tttt           tt    tttt"
		"tttttttt        tttt   tttt tt"
		"tttt     ttt   ttt          tt";

	texture = Texture2D::Create("Assets/Textures/RPGpack_sheet_2X.png");
	tree = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 4,1 }, { 128.f,128.f }, {1,2});
	mud = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 6,11 }, { 128.f,128.f });
	land = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 3,10 }, { 128.f,128.f });
	water = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 11,11 }, { 128.f,128.f });

	asset_map['l'] = land;
	asset_map['w'] = water;
	asset_map['t'] = tree;
	asset_map['m'] = mud;

	m_FrameBuffer = FrameBuffer::Create({1920,1080});//create a frame buffer object

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
		m_FrameBuffer->Bind();//Bind the frame buffer so that it can store the pixel data to a texture

		RenderCommand::ClearColor(glm::vec4(0.2));
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

	}
		/*
		Set the model transform. for now there is no scale or rotation
		just multiply the scale and rotation matrix with position to get the full transform
		*/
	
		Renderer2D::BeginScene(m_camera.GetCamera());
		for (int i = 0; i < level_map.size(); i++) {
		
			ref<SubTexture2D> subTexture;
			subTexture = asset_map[level_map[i]];
			if (subTexture)
				Renderer2D::DrawSubTexturedQuad({ i % 30,i / 30,0 }, { 1,1,1 }, subTexture);
		}
		Renderer2D::EndScene();

		Renderer2D::BeginScene(m_camera.GetCamera());
		for (int i = 0; i < tree_map.size(); i++) {

			ref<SubTexture2D> subTexture;
			subTexture = asset_map[tree_map[i]];
			if (subTexture)
				Renderer2D::DrawSubTexturedQuad({ i % 30,i / 30, 0.1 }, { 1,1,1 }, subTexture);
		}
		Renderer2D::EndScene();
		m_FrameBuffer->UnBind();
}

void SandBox2dApp::OnImGuiRender()
{
	HZ_PROFILE_SCOPE("ImGUI RENDER");

	ImGui::DockSpaceOverViewport();//always keep DockSpaceOverViewport() above all other ImGui windows to make the other windows docable
	ImGui::Begin("Color");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();
	ImGui::Begin("Multi");

	ImGui::Image((void *)m_FrameBuffer->GetSceneTextureID(), { 1920,1080 });
	ImGui::End();
}

void SandBox2dApp::OnEvent(Event& e)
{
	m_camera.OnEvent(e);
}
