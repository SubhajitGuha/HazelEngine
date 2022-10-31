#include "HazelEditor.h"
//#include "Hazel/Profiling.h"

 HazelEditor:: HazelEditor()
	:Layer("Renderer2D layer"), m_camera(1920.0 / 1080.0)
{
	//HZ_PROFILE_SCOPE(" HazelEditor:: HazelEditor()");

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

void  HazelEditor::OnAttach()
{
	m_scene = Scene::Create();

	Square_entity = m_scene->CreateEntity("Square");
	auto transform = glm::translate(glm::mat4(1.f), { 1,0,0 }) * glm::rotate(glm::mat4(1.f), glm::radians(19.f), { 0,0,1 }) * glm::scale(glm::mat4(1.f), glm::vec3(3));
	Square_entity->AddComponent<TransformComponent>(transform);
	Square_entity->AddComponent<CameraComponent>();

	camera_entity = m_scene->CreateEntity("Camera");//create the camera entity
	auto transform1 = glm::translate(glm::mat4(1.f), { 0,2,0 }) * glm::rotate(glm::mat4(1.f), glm::radians(0.f), { 0,0,1 }) * glm::scale(glm::mat4(1.f), glm::vec3(3));
	camera_entity->AddComponent<TransformComponent>(transform1);
	camera_entity->AddComponent<CameraComponent>(120 , 80);

	camera_entity->GetComponent<CameraComponent>().camera.bIsMainCamera = false;

	m_scene->print();
}

void  HazelEditor::OnDetach()
{
}

void  HazelEditor::OnUpdate(float deltatime )
{

	HZ_PROFILE_SCOPE(" HazelEditor::OnUpdate");
	{
		m_FrameBuffer->Bind();//Bind the frame buffer so that it can store the pixel data to a texture

		RenderCommand::ClearColor({ 0.5,0.3,0.8,1 });
		RenderCommand::Clear();

		if (isWindowFocused)//Take inputs only when the window is focused
		{
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
	}
		/*
		Set the model transform. for now there is no scale or rotation
		just multiply the scale and rotation matrix with position to get the full transform
		*/
	
		//Renderer2D::BeginScene(m_camera.GetCamera());
		//for (int i = 0; i < level_map.size(); i++) {
		//
		//	ref<SubTexture2D> subTexture;
		//	subTexture = asset_map[level_map[i]];
		//	if (subTexture)
		//		Renderer2D::DrawSubTexturedQuad({ i % 30,i / 30,0 }, { 1,1,1 }, subTexture);
		//}
		//Renderer2D::EndScene();
		//
		//Renderer2D::BeginScene(m_camera.GetCamera());
		//for (int i = 0; i < tree_map.size(); i++) {
		//
		//	ref<SubTexture2D> subTexture;
		//	subTexture = asset_map[tree_map[i]];
		//	if (subTexture)
		//		Renderer2D::DrawSubTexturedQuad({ i % 30,i / 30, 0.1 }, { 1,1,1 }, subTexture);
		//}
		//Renderer2D::EndScene();
		//Renderer2D::BeginScene(m_camera.GetCamera());
		//Renderer2D::DrawQuad(position, glm::vec3(scale), Color1);
		//Renderer2D::EndScene();

		////using Entt (entity component system)
		//Renderer2D::BeginScene(Square_entity->GetComponent<CameraComponent>());
		//Renderer2D::DrawQuad(Square_entity->GetComponent<TransformComponent>(), { 0,0.6,0.9,1 });
		//Renderer2D::EndScene();
		m_scene->OnUpdate(deltatime);
		m_scene->Resize(m_ViewportSize.x,m_ViewportSize.y);
		m_FrameBuffer->UnBind();
}

void  HazelEditor::OnImGuiRender()
{
	HZ_PROFILE_SCOPE("ImGUI RENDER");

	ImGui::DockSpaceOverViewport();//always keep DockSpaceOverViewport() above all other ImGui windows to make the other windows docable
	ImGui::Begin("Color");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();

	ImGui::Begin("Viewport");
	isWindowFocused = ImGui::IsWindowFocused();
	ImVec2 Size = ImGui::GetContentRegionAvail();
	if (m_ViewportSize != *(glm::vec2*)&Size)
	{
		m_ViewportSize = { Size.x,Size.y };
		m_FrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
	}
	ImGui::Image((void*)m_FrameBuffer->GetSceneTextureID(), *(ImVec2*)&m_ViewportSize);
	ImGui::End();

	ImGui::Begin("Camera Selection");
	ImGui::Checkbox("Camera1 : is Main Camera", &IsMainCamera);
	auto cam1 = m_scene->GetEntitybyName("Camera");
	cam1->GetComponent<CameraComponent>().camera.bIsMainCamera = IsMainCamera;

	bool val2 = true;
	ImGui::Checkbox("Camera2 : is Main Camera", &IsMainCamera2);
	auto cam2 = m_scene->GetEntitybyName("Square");
	cam2->GetComponent<CameraComponent>().camera.bIsMainCamera = IsMainCamera2;
	ImGui::End();
}

void  HazelEditor::OnEvent(Event& e)
{
	if (isWindowFocused)
		m_camera.OnEvent(e);
}
