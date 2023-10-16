#include "hzpch.h"
#include "Scene.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Entity.h"
#include "Component.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/Camareas/EditorCamera.h"
#include "glad/glad.h"
#include "Hazel/LoadMesh.h"
#include "PointLight.h"
#include "Hazel/Physics/Physics3D.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Renderer/Atmosphere.h"
#include "Hazel/Renderer/SkyRenderer.h"
#include "Hazel/Renderer/Terrain.h"

namespace Hazel {
	
	bool Scene::TOGGLE_SHADOWS = true;
	bool Scene::TOGGLE_SSAO = true;
	float Scene::foliage_dist = 3000;
	float Scene::num_foliage = 10000;
	//std::vector<PointLight*> Scene::m_PointLights;
	EditorCamera editor_cam;
	 LoadMesh* Scene::Sphere=nullptr, *Scene::Sphere_simple = nullptr, *Scene::Cube= nullptr, *Scene::Plane= nullptr
		 , *Scene::plant, *Scene::House,*Scene::Windmill, *Scene::Fern, *Scene::Sponza, *Scene::Grass, *Scene::Flower, *Scene::Tree;
	 bool capture = false;
	 glm::vec3 camloc = { 0,0,0 }, camrot = {0,0,0};
	Scene::Scene()
	{
		//framebuffer = FrameBuffer::Create({ 2048,2048 });
		Physics3D::Initilize();
		SkyRenderer::Initilize();
		SkyRenderer::SetSkyType(SkyType::PROCEDURAL_SKY);

		Renderer3D::Init();
		Renderer2D::Init();

		auto viewportSize = RenderCommand::GetViewportSize();

		Flower = new LoadMesh("Assets/Meshes/flower.fbx");
		Sphere = new LoadMesh("Assets/Meshes/Sphere.fbx");
		Sphere_simple = new LoadMesh("Assets/Meshes/sphere_simple.fbx");
		Plane = new LoadMesh("Assets/Meshes/Plane.fbx");
		Cube = new LoadMesh("Assets/Meshes/Cube.fbx");
		Fern = new LoadMesh("Assets/Meshes/HZ_Pine1.fbx");
		Grass = new LoadMesh("Assets/Meshes/grass.fbx");
		plant = new LoadMesh("Assets/Meshes/ZombiePlant.fbx");
		House = new LoadMesh("Assets/Meshes/cityHouse_Unreal.fbx");
		Tree = new LoadMesh("Assets/Meshes/Tree.fbx");
		Windmill = new LoadMesh("Assets/Meshes/Windmill.fbx");
		Sponza = new LoadMesh("Assets/Meshes/Sponza.fbx");
		Renderer3D::SetUpCubeMapReflections(*this);
		editor_cam.SetPerspectiveFar(10000);
		
		m_Terrain = std::make_shared<Terrain>(2048,2048);
		//initilize Bloom
		m_Bloom = Bloom::Create();
		m_Bloom->GetFinalImage(0, { 1920,1080 });
		m_Bloom->InitBloom();
	}
	Scene::~Scene()
	{
	}
	Entity* Scene::CreateEntity(const std::string& name)
	{
		m_entity = m_registry.create();
		auto entity = new Entity( this,m_entity);
		entity->AddComponent<TransformComponent>();
		entity->AddComponent<StaticMeshComponent>(Cube);
		if (name == "")//if no name is give to an entity just call it entity (i.e define tag with entity)
			entity->AddComponent<TagComponent>("Entity");//automatically add a tag component when an entity is created
		else
			entity->AddComponent<TagComponent>(name);
		//Entity_Map[entity->GetComponent<TagComponent>()] = entity;
		capture = true;
		return entity;
	}
	void Scene::DestroyEntity(const Entity& delete_entity)
	{
		m_registry.destroy(delete_entity);
	}
	void Scene::OnUpdate(TimeStep ts)
	{
		MainCamera = nullptr;//if there is no main camera Then dont render
		
		//update camera , Mesh Forward vectors....
		auto view = m_registry.view<CameraComponent>();
		for (auto entt : view) {
			auto& camera = m_registry.get<CameraComponent>(entt);
			if (camera.camera.bIsMainCamera) {
				MainCamera = (&camera.camera);
				auto& tc = m_registry.get<TransformComponent>(entt);
				auto& transform = tc.GetTransform();
				if (camera.bFollowPlayer)
				{
					auto& rotation = tc.Rotation;
					auto& cam_pos = MainCamera->GetCameraPosition();

					tc.RightVector = glm::cross(tc.ForwardVector, tc.UpVector);
					tc.ForwardVector = glm::mat3(glm::rotate(glm::radians(rotation.y), tc.UpVector)) * glm::mat3(glm::rotate(glm::radians(rotation.x), tc.RightVector)) * glm::vec3(0, 0, 1);
					MainCamera->SetViewDirection(tc.ForwardVector);// Make the view direction of the camera same as the mesh forward direction

					float dist = glm::length(camera.camera_dist);//scale the -forward vector with the radius of the circle
					auto& object_pos = glm::vec3(transform[3][0], transform[3][1], transform[3][2]);
					tc.ForwardVector = -glm::normalize(tc.ForwardVector);
					MainCamera->SetCameraPosition(tc.ForwardVector * dist + object_pos-camera.camera_dist);//align the camera with the mesh view vector
				}
				break;
			}
		}

		if (!MainCamera)
		{
			MainCamera = &editor_cam;
			//editcam = editor_cam;
		}

		SkyRenderer::RenderSky(*MainCamera);
		
		MainCamera->OnUpdate(ts);
		//run scripts
		m_registry.view<ScriptComponent>().each([=](entt::entity entity, ScriptComponent& nsc) 
		{
				if(nsc.m_Script==nullptr)
				nsc.CreateInstance();// this needs to be done once ,not every frame.
			
				m_registry.each([&](auto m_entity)//iterate through all entities
					{
						if (entity == m_entity)
						{
							nsc.m_Script->m_Entity = new Entity(this, m_entity);//the Entity in the script class is made equal to the created scene entity
						}
					});
			nsc.m_Script->OnUpdate(ts);//update to get the script values
			});
		
		if (m_PointLights.size() > 0)
			Renderer3D::SetPointLightPosition(m_PointLights);

		////debug physics
		//Renderer2D::BeginScene(*MainCamera);
		//for (int i = 0; i < Physics3D::DebugPoints.size(); i++)
		//{
		//	Renderer2D::DrawLine(Physics3D::DebugPoints[i].pos0, Physics3D::DebugPoints[i].pos1, glm::vec4(0.0, 1.0, 0.6, 1.0));
		//}

		Renderer3D::SetSunLightDirection(Renderer3D::m_SunLightDir);
		Renderer3D::SetSunLightColorAndIntensity(Renderer3D::m_SunColor, Renderer3D::m_SunIntensity);

		//m_Terrain->RenderTerrain(*MainCamera);
		
		Renderer3D::RenderScene_Deferred(this);
	}
	void Scene::OnCreate()
	{
	}
	void Scene::Resize(float Width, float Height)
	{
		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view) {
			auto camera = m_registry.get<CameraComponent>(entity).camera;
			if(camera.IsResiziable && camera.IsResiziable)
				m_registry.get<CameraComponent>(entity).camera.SetViewportSize(Width/Height);
		}
		MainCamera->SetViewportSize(Width/ Height);//resize the editor camera
	}
	void Scene::OnEvent(Event& e)
	{
		MainCamera->OnEvent(e);

		for (auto s : m_scriptsMap)
		{
			s.second->OnEvent(e);
		}
	}

	void Scene::PostProcess()
	{
		Atmosphere::RenderAtmosphere(*MainCamera, 200.0f);
	}

	void Scene::AddPointLight(PointLight* light)
	{
		m_PointLights.push_back(light);
	}

	ref<Scene> Scene::Create()
	{
		return std::make_shared<Scene>();
	}
}