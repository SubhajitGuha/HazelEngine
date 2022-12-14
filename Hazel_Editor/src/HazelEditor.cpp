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
	 camera_entity->AddComponent<CameraComponent>();

	 camera_entity->GetComponent<CameraComponent>().camera.bIsMainCamera = false;
	 camera_entity->GetComponent<CameraComponent>().camera.SetOrthographic(50);
//....................script......................................................................

	 class CustomScript :public ScriptableEntity {
	 public:
		 glm::vec3 position = { 0,0,0 };
		 float rotate = 0.0;
		 float ObjSpeed = 10;
		 float scale = 1;
		 float size = 1.0f;
		 virtual void OnUpdate(TimeStep ts) override 
		 { 
			 if (!m_Entity)
				 return;

			 m_Entity->m_DefaultColor = glm::vec4(0.8, 0.8, 0.1, 1.0);
			 
			 if (Input::IsKeyPressed(HZ_KEY_E))
				 rotate += 1;
			 if (Input::IsKeyPressed(HZ_KEY_Q))
				 rotate -= 1;
			 if (Input::IsKeyPressed(HZ_KEY_UP))
				 position.y += ObjSpeed * ts;
			 if (Input::IsKeyPressed(HZ_KEY_DOWN))
				 position.y -= ObjSpeed * ts;
			 if (Input::IsKeyPressed(HZ_KEY_LEFT))
				 position.x -= ObjSpeed * ts;
			 if (Input::IsKeyPressed(HZ_KEY_RIGHT))
				 position.x += ObjSpeed * ts;
			 if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_4))
				 scale += 0.01;
			 if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_5))
				 scale -= 0.01;
			 if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_1))
				 size += 0.1;
			 if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_2))
				 size -= 0.1;
			 auto transform = glm::translate(glm::mat4(1.f), position) * glm::rotate(glm::mat4(1.0f),glm::radians(rotate),glm::vec3(0,0,1)) * glm::scale(glm::mat4(1.f), glm::vec3(scale));
			 m_Entity->ReplaceComponent<TransformComponent>(transform);//controlling transform
			 m_Entity->GetComponent<CameraComponent>().camera.SetOrthographic(size);//controlling camera
		 }
		 virtual void OnCreate() override{}
		 virtual void OnDestroy() override{}
	 };
	 //.........................script.........................................................
	 Square_entity->AddComponent<ScriptComponent>().Bind<CustomScript>();
 }

void  HazelEditor::OnDetach()
{
}

void  HazelEditor::OnUpdate(float deltatime )
{

	HZ_PROFILE_SCOPE(" HazelEditor::OnUpdate");
	{
		m_FrameBuffer->Bind();//Bind the frame buffer so that it can store the pixel data to a texture

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

		////using Entt (entity component system)
		//Renderer2D::BeginScene(Square_entity->GetComponent<CameraComponent>());
		//Renderer2D::DrawQuad(Square_entity->GetComponent<TransformComponent>(), { 0,0.6,0.9,1 });
		//Renderer2D::EndScene();
	//m_scene->OnUpdate(deltatime);

	auto velocity = [&](const glm::vec2& point1, const glm::vec2& point2,float& s) {
		glm::vec2 vel;
		vel = point2 - point1;
		vel.x *= s;
		vel.y *= s;
		return vel;
	};
	
	std::vector<glm::vec2> arr_pts = { { 5,-5 },{8,-8},{10,0} ,{12,3},{15,2},{16,3},{18,2.1},{18.2,3.5},{19,3.9},{19.8,-5},{20.5,2} };//array of points for testing
	glm::vec2 tmp_point = P1 , tmp_vel = velocity(P0,arr_pts[0],factor);

	Renderer2D::LineBeginScene(m_camera.GetCamera());
	for (int i=0;i<arr_pts.size()-1;i++)
	{	
		auto x = velocity(tmp_point, arr_pts[i + 1], factor);
		Renderer2D::DrawCurve(tmp_point, arr_pts[i], tmp_vel, x);//drawing a hermitian curve
		tmp_point = arr_pts[i];
		tmp_vel = x;
		Renderer2D::Draw_Bezier_Curve(P0, c1, c2, P1);
	}
	Renderer2D::LineEndScene();
	
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

	ImGui::Begin("Curve Controller");
	ImGui::DragFloat2("control_point0", (float*)&c1,0.10,-100.0,100.0);
	ImGui::DragFloat2("control_point1", (float*)&c2, 0.10, -100.0, 100.0);
	ImGui::DragFloat2("point2", (float*)&P2, 1.0, -100.0, 100.0);
	ImGui::DragFloat2("point3", (float*)&P3, 1.0, -100.0, 100.0);
	ImGui::DragFloat("Factor", &factor, 0.01, 0.0, 1.0);
	ImGui::End();
}

void  HazelEditor::OnEvent(Event& e)
{
	if (isWindowFocused)
		m_camera.OnEvent(e);
}
