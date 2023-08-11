#include <hzpch.h>
#include "HazelEditor.h"
#include "Hazel//Renderer/Shadows.h"
#include "Hazel/Physics/Physics3D.h"
#include "Hazel/Renderer/Terrain.h"

//#include "Hazel/Profiling.h"

LoadMesh* mesh;
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
		"lllwwwwwwwwmmmmwwwwwwwwmwwllll"
		"llllllwmwwwwwwwwwwwwwwwlllllll"
		"llllwwwwwwwwwwwwwwwwllllllllll"
		"lllllllllwwwwwwwwwwwllllllllll"
		"llllllllmmmwwwwwllllllllllllll"
		"llllllllllllllllllllllllllllll";

	tree_map = 
		"ttttt  ttttt  tttt  ttttt  ttt"
		"ttt t     t     ttttttt tt ttt"
		"ttt  tttt          ttttttttttt"
		"tt  tt                 ttt   t"
		"tttt                     ttttt"
		"ttt                        ttt"
		"tttttt                 ttt  tt"
		"tttt                t tt    tt"
		"     tttt           tt    tttt"
		"tttt ttt        tttt   tttt tt"
		"tttt     ttt   ttt          tt";


	texture = Texture2D::Create(std::string("Assets/Textures/RPGpack_sheet_2X.png"));
	tree = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 4,1 }, { 128.f,128.f }, {1,2});
	mud = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 6,11 }, { 128.f,128.f });
	land = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 3,10 }, { 128.f,128.f });
	water = SubTexture2D::CreateFromCoordinate(texture, { 2560.f,1664.f }, { 11,11 }, { 128.f,128.f });

	asset_map['l'] = land;
	asset_map['w'] = water;
	asset_map['t'] = tree;
	asset_map['m'] = mud;

	m_FrameBuffer = FrameBuffer::Create({1920,1080});//create a frame buffer object
	m_FrameBuffer2 = FrameBuffer::Create({ 1920,1080 });
}

 void  HazelEditor::OnAttach()
 {
	 m_scene = Scene::Create();

	//Square_entity = m_scene->CreateEntity("Square");
	//
	//Square_entity->AddComponent<TransformComponent>(glm::vec3(5,-2,0));
	//Square_entity->AddComponent<CameraComponent>();
	//Square_entity->AddComponent<SpriteRenderer>(glm::vec4(1, 0.2, 0, 1));
	//
	// camera_entity = m_scene->CreateEntity("Camera");//create the camera entity
	// camera_entity->AddComponent<TransformComponent>(glm::vec3(-6,-1,0));
	// camera_entity->AddComponent<CameraComponent>();
	//
	// camera_entity->GetComponent<CameraComponent>().camera.bIsMainCamera = false;
	// camera_entity->GetComponent<CameraComponent>().camera.SetOrthographic(50);
	//
	// Square2 = m_scene->CreateEntity("2ndSquare");
	// Square2->AddComponent<TransformComponent>(glm::vec3(-1,0,5));
	// Square2->AddComponent<CameraComponent>();
	// //Square2->GetComponent<CameraComponent>();
	//
	// Square3 = m_scene->CreateEntity("3rdSquare");
	// Square3->AddComponent<TransformComponent>(glm::vec3(-2,0,-5));
//....................script......................................................................

	 //.........................script.........................................................
	//Square_entity->AddComponent<ScriptComponent>().Bind<(CustomScript)>();
	 //Square3->AddComponent<ScriptComponent>().Bind<CustomScript>();
	//Square3->AddComponent<CameraComponent>();
	 //HAZEL_WARN(typeid(CustomScript).raw_name());
	 m_Pannel.Context(m_scene);
 }

void  HazelEditor::OnDetach()
{
}

void  HazelEditor::OnUpdate(float deltatime )
{
	frame_time = deltatime;
	HZ_PROFILE_SCOPE(" HazelEditor::OnUpdate");
	{
		m_FrameBuffer2->Bind();//Bind the frame buffer so that it can store the pixel data to a texture

		RenderCommand::ClearColor({0,0,0,1});
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
		//using Entt (entity component system)
		//Renderer2D::BeginScene(Square_entity->GetComponent<CameraComponent>());
		//Renderer2D::DrawQuad(Square_entity->GetComponent<TransformComponent>(), { 0,0.6,0.9,1 });
		//Renderer2D::EndScene();

	m_scene->OnUpdate(deltatime);
	m_scene->Resize(m_ViewportSize.x, m_ViewportSize.y);
	m_FrameBuffer2->UnBind();

	m_FrameBuffer2->BindFramebufferTexture(ORIGINAL_SCENE_TEXTURE_SLOT);
	m_FrameBuffer2->BindFramebufferTexture(SCENE_TEXTURE_SLOT);
	m_FrameBuffer2->BindFramebufferDepthTexture(SCENE_DEPTH_SLOT);
	m_scene->m_Bloom->GetFinalImage(m_FrameBuffer2->GetSceneTextureID(), RenderCommand::GetViewportSize());
	
	m_scene->m_Bloom->RenderBloomTexture();
	
	m_FrameBuffer->Bind();
	RenderCommand::ClearColor({ 1,0,0,1 });
	RenderCommand::Clear();
	//m_scene->PostProcess();
	m_scene->m_Bloom->Update(deltatime);
	m_FrameBuffer->UnBind();
}

void  HazelEditor::OnImGuiRender()
{
	HZ_PROFILE_SCOPE("ImGUI RENDER");

	ImGui::DockSpaceOverViewport();//always keep DockSpaceOverViewport() above all other ImGui windows to make the other windows docable

	ImGui::Begin("Viewport");
	isWindowFocused = ImGui::IsWindowFocused();
	ImVec2 Size = ImGui::GetContentRegionAvail();
	if (m_ViewportSize != *(glm::vec2*)&Size)
	{
		m_ViewportSize = { Size.x,Size.y };
		m_FrameBuffer2->Resize(m_ViewportSize.x, m_ViewportSize.y);
		m_FrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
		m_camera.onResize(Size.x, Size.y);
	}
	ImGui::Image((void*)m_FrameBuffer->GetSceneTextureID(), *(ImVec2*)&m_ViewportSize);
	ImGui::End();


	ImGui::Begin("Light controller");
	ImGui::DragFloat3("Sun Direction", (float*)&Renderer3D::m_SunLightDir,0.01);
	ImGui::ColorEdit3("Sun Light Color", glm::value_ptr(Renderer3D::m_SunColor));
	ImGui::DragFloat("Sun Intensity", &Renderer3D::m_SunIntensity, 0.01);

	ImGui::DragInt("Number Of Mips", &Bloom::NUMBER_OF_MIPS);
	ImGui::DragInt("Filter Radius", &Bloom::FILTER_RADIUS);
	ImGui::DragFloat("Exposure", &Bloom::m_Exposure,0.01);
	ImGui::DragFloat("Bloom Amount", &Bloom::m_BloomAmount, 0.01);
	ImGui::DragFloat("Brightness Threshold", &Bloom::m_BrightnessThreshold, 0.001);

	ImGui::End();

	ImGui::Begin("Shadow Map and SSAO map");
	ImGui::Text("SHADOW MAP");
	ImGui::Image((void*)Renderer3D::depth_id, ImVec2(512, 512));
	ImGui::DragInt("Cascade Level", &Shadows::Cascade_level, 1, 0, 100);
	ImGui::DragFloat("lamda", &Shadows::m_lamda, 0.00001, 0, 1,"%8f");
	ImGui::Text("SSAO MAP");
	ImGui::Image((void*)Renderer3D::ssao_id, ImVec2(512, 512));
	ImGui::End();

	ImGui::Begin("Benchmark");
	ImGui::Text("FPS :  ");
	ImGui::SameLine();
	ImGui::TextColored({ 1,0.2,1,1 }, std::to_string(1.0f/frame_time).c_str());
	//ImGui::NewLine();
	ImGui::Text("frame time : ");
	ImGui::SameLine();
	ImGui::TextColored({ 0,1,0,1 }, std::to_string(frame_time).c_str());
	ImGui::Checkbox("Simulate Physics", &Physics3D::SimulatePhysics);
	ImGui::DragFloat("Foliage Coverage", &Scene::foliage_dist, 100, 0, 100000, "%8f");
	ImGui::DragFloat("Foliage Density", &Scene::num_foliage, 100, 0, 100000, "%8f");
	ImGui::End();
	m_Pannel.OnImGuiRender();
}

void  HazelEditor::OnEvent(Event& e)
{
	if (isWindowFocused) {
		m_camera.OnEvent(e);
		m_scene->OnEvent(e);
	}
}
