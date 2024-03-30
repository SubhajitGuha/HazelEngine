#include <hzpch.h>
#include "HazelEditor.h"
#include "ImGuizmo.h"
#include "Hazel//Renderer/Shadows.h"
#include "Hazel/Physics/Physics3D.h"
#include "Hazel/Renderer/DeferredRenderer.h"
#include "Hazel/Renderer/Terrain.h"
#include "Hazel/Renderer/FoliageRenderer.h"
#include "Hazel/Renderer/SkyRenderer.h"
#include "Hazel/Renderer/Fog.h"
#include "Hazel/RayTracer/RayTracer.h"

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

	auto viewportSize = RenderCommand::GetViewportSize();
	m_FrameBuffer = FrameBuffer::Create({ (unsigned int)viewportSize.x,(unsigned int)viewportSize .y});//create a frame buffer object
	m_FrameBuffer2 = FrameBuffer::Create({ (unsigned int)viewportSize.x,(unsigned int)viewportSize.y });
	m_FrameBuffer3 = FrameBuffer::Create({ (unsigned int)viewportSize.x,(unsigned int)viewportSize.y });

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
	numFrame++;
	frame_time = deltatime;
	m_scene->OnUpdate(deltatime);
	Renderer3D::ForwardRenderPass(m_scene.get());//forward pass for later deferred stage

	m_FrameBuffer2->Bind();//Bind the frame buffer so that it can store the pixel data to a texture
	RenderCommand::ClearColor({ 0,0,0,1 });
	RenderCommand::Clear();
	Renderer3D::RenderScene_Deferred(m_scene.get()); //only do the deferred rendering here to capture it in fb
	m_scene->Resize(m_ViewportSize.x, m_ViewportSize.y);
	m_FrameBuffer2->UnBind();

	m_FrameBuffer2->BindFramebufferTexture(ORIGINAL_SCENE_TEXTURE_SLOT); //copy the rendered image to these slots
	m_FrameBuffer2->BindFramebufferTexture(SCENE_TEXTURE_SLOT);
	Renderer3D::RenderWithAntialiasing();

	//do post processing stuff here
	m_scene->m_Fog->RenderFog(*m_scene->GetCamera(), RenderCommand::GetViewportSize());
	m_scene->m_Bloom->GetFinalImage(m_FrameBuffer2->GetSceneTextureID(), RenderCommand::GetViewportSize());	
	m_scene->m_Bloom->RenderBloomTexture();
	
	m_FrameBuffer->Bind();
	RenderCommand::ClearColor({ 0,0,0,1 });
	RenderCommand::Clear();
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
		m_FrameBuffer3->Resize(m_ViewportSize.x, m_ViewportSize.y);
		m_FrameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
		numFrame = 0;
		m_camera.onResize(Size.x, Size.y);
	}
	ImGui::Image(reinterpret_cast<void*>(m_FrameBuffer->GetSceneTextureID()), *(ImVec2*)&m_ViewportSize);

	if (SceneHierarchyPannel::m_selected_entity) //gizmo logics
	{
		auto w_pos = ImGui::GetWindowPos();
		auto& transform = SceneHierarchyPannel::m_selected_entity->GetComponent<TransformComponent>();
		ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
		if (Input::IsKeyPressed(HZ_KEY_E))
			transform.mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if(Input::IsKeyPressed(HZ_KEY_R))
			transform.mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (Input::IsKeyPressed(HZ_KEY_T))
			transform.mCurrentGizmoOperation = ImGuizmo::SCALE;
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(w_pos.x,w_pos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
		//ImGuizmo::DrawGrid(&m_scene->GetCamera()->GetViewMatrix()[0][0], &m_scene->GetCamera()->GetProjectionMatrix()[0][0], &transform.GetTransform()[0][0], 100);
		glm::mat4 trans = (transform.GetTransform());
		glm::mat4 camera_view = (m_scene->GetCamera()->GetViewMatrix());
		glm::mat4 proj = (m_scene->GetCamera()->GetProjectionMatrix());
		ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(proj), transform.mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(trans));
		if (ImGuizmo::IsUsing())
		{
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(trans), glm::value_ptr(transform.Translation), glm::value_ptr(transform.Rotation),
				glm::value_ptr(transform.Scale));
		}
	}
	ImGui::End();

	ImGui::Begin("Viewport_RayTracer");
	isWindowFocused = ImGui::IsWindowFocused();
	RayTracer::isViewportFocused = isWindowFocused;
	ImVec2 Size2 = ImGui::GetContentRegionAvail();
	if (m_RTViewportSize != *(glm::vec2*)&Size2)
	{
		m_RTViewportSize = { Size2.x,Size2.y };
		m_scene->m_rayTracer->Resize(static_cast<int>(m_RTViewportSize.x), static_cast<int>(m_RTViewportSize.y));
	}
	ImGui::Image(reinterpret_cast<void*>(RayTracer::m_Output_TextureID), *(ImVec2*)glm::value_ptr(m_ViewportSize), { 0,1 }, {1,0});
	ImGui::End();

	ImGui::Begin("RayTracer Control");
	ImGui::Checkbox("RenderSky?", &RayTracer::EnableSky);
	ImGui::Checkbox("Denoise", &RayTracer::Denoise);
	if (ImGui::Button("Update Materials"))
		m_scene->m_rayTracer->bvh->UpdateMaterials();
	ImGui::DragInt("Num Bounces", &RayTracer::numBounces);
	ImGui::DragInt("Samples Per Pixel", &RayTracer::samplesPerPixel);

	ImGui::DragFloat3("LightPos", (float*)glm::value_ptr(RayTracer::m_LightPos),0.01);
	ImGui::DragFloat(" LightStrength ", &RayTracer::m_LightStrength, 0.01);
	
	ImGui::End();

	ImGui::Begin("Light controller");
	ImGui::DragFloat("Camera Speed", &EditorCamera::camera_MovementSpeed);
	ImGui::DragFloat3("Sun Direction", (float*)&Renderer3D::m_SunLightDir,0.01);
	ImGui::ColorEdit3("Sun Light Color", glm::value_ptr(Renderer3D::m_SunColor));
	ImGui::DragFloat("Sun Intensity", &Renderer3D::m_SunIntensity, 0.01);

	ImGui::DragInt("Number Of Mips", &Bloom::NUMBER_OF_MIPS);
	ImGui::DragInt("Filter Radius", &Bloom::FILTER_RADIUS);
	ImGui::DragFloat("Exposure", &Bloom::m_Exposure,0.01);
	ImGui::DragFloat("Bloom Amount", &Bloom::m_BloomAmount, 0.01);
	ImGui::DragFloat("Brightness Threshold", &Bloom::m_BrightnessThreshold, 0.001);
	ImGui::End();

	ImGui::Begin("Terrain Params");
	ImGui::DragFloat("Water Level", &Terrain::WaterLevel, 0.001);
	ImGui::DragFloat("Hill Level", &Terrain::HillLevel, 0.001);
	ImGui::DragFloat("Mountain Level", &Terrain::MountainLevel, 0.001);
	ImGui::DragFloat("FoliageHeight", &Terrain::FoliageHeight, 0.1);
	ImGui::DragFloat("Terrain Scale", &Terrain::HeightScale, 10);
	ImGui::Checkbox("Show Terrain", &Terrain::bShowTerrain);
	ImGui::Checkbox("Show Wireframe", &Terrain::bShowWireframeTerrain);
	ImGui::Text(std::to_string(Terrain::ChunkIndex).c_str());
	ImGui::End();

	ImGui::Begin("Buffers");
	ImGui::Text("SHADOW MAP");
	ImGui::DragInt("Index", &Renderer3D::index, 1.0, 0, 3);
	ImGui::Image((void*)Renderer3D::depth_id[Renderer3D::index], ImVec2(512, 512), { 0,1 }, { 1,0 });
	ImGui::DragInt("Cascade Level", &Shadows::Cascade_level, 1, 0, 100);
	ImGui::DragFloat("lamda", &Shadows::m_lamda, 0.00001, 0, 1,"%8f");
	ImGui::Text("SSAO MAP");
	ImGui::Image((void*)Renderer3D::ssao_id, ImVec2(512, 512), { 0,1 }, {1,0});	
	ImGui::Text("Normal map");
	ImGui::Image((void*)DefferedRenderer::GetBuffers(0), ImVec2(512, 512), { 0, 1 }, { 1,0 });
	ImGui::Text("Velocity Buffer For TAA");
	ImGui::Image((void*)DefferedRenderer::GetBuffers(1), ImVec2(512, 512), { 0, 1 }, { 1,0 });
	ImGui::Text("diffuse");
	ImGui::Image((void*)DefferedRenderer::GetBuffers(2), ImVec2(512, 512), { 0, 1 }, { 1,0 });
	ImGui::Text("Roughness metallic");
	ImGui::Image((void*)DefferedRenderer::GetBuffers(3), ImVec2(512, 512), { 0, 1 }, { 1,0 });
	ImGui::Text("Density Map");
	ImGui::Image((void*)Foliage::m_DensityMapID, ImVec2(512, 512), { 0, 1 }, { 1,0 });

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
	
	ImGui::DragFloat("FOG DENSITY", &m_scene->fogDensity, 0.00001);
	ImGui::DragFloat("FOG END", &m_scene->fogEnd, 0.1);
	ImGui::DragFloat("FOG TOP", &m_scene->fogTop, 0.1);
	ImGui::ColorEdit3("FOG COLOR", glm::value_ptr(m_scene->fogColor));

	ImGui::End();
	m_Pannel.OnImGuiRender();
	m_ContentBrowser.OnImGuiRender();
	m_MaterialEditor.OnImGuiRender();
}

void  HazelEditor::OnEvent(Event& e)
{
	if (isWindowFocused) {
		m_camera.OnEvent(e);
		m_scene->OnEvent(e);
	}
}
