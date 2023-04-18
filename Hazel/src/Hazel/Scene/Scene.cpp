#include "hzpch.h"
#include "Scene.h"
#include "Entity.h"
#include "Component.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/Camareas/EditorCamera.h"
#include "glad/glad.h"
#include "Hazel/LoadMesh.h"
#include "PointLight.h"


namespace Hazel {
	
	//std::vector<PointLight*> Scene::m_PointLights;
	 LoadMesh* Scene::Sphere=nullptr, *Scene::Cube= nullptr, *Scene::Plane= nullptr, *Scene::plant, *Scene::House,*Scene::Windmill, *Scene::Fern, *Scene::Sponza;
	 EditorCamera editor_cam;
	 bool capture = false;
	Scene::Scene()
	{
		//framebuffer = FrameBuffer::Create({ 2048,2048 });

		Sphere = new LoadMesh("Assets/Meshes/Sphere.fbx");
		Plane = new LoadMesh("Assets/Meshes/Plane.fbx");
		Cube = new LoadMesh("Assets/Meshes/Cube.fbx");
		Fern = new LoadMesh("Assets/Meshes/shrub.fbx");
		plant = new LoadMesh("Assets/Meshes/ZombiePlant.fbx");
		House = new LoadMesh("Assets/Meshes/cityHouse_Unreal.fbx");
		Windmill = new LoadMesh("Assets/Meshes/Windmill.fbx");
		Sponza = new LoadMesh("Assets/Meshes/Sponza.fbx");

		editor_cam.SetViewportSize(1920.0,1080.0);
		Renderer3D::SetUpCubeMapReflections(*this);
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
		editor_cam.OnUpdate(ts);
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

		Camera* MainCamera=nullptr;//if there is no main camera Then dont render
		{
			auto view = m_registry.view<CameraComponent>();
			for (auto entt : view) {
				auto& camera = m_registry.get<CameraComponent>(entt);
				if (camera.camera.bIsMainCamera)
					MainCamera = &camera.camera;
			}
		}
		//Renderer3D::AmbiantOcclusion(*this, editor_cam);
		
		m_registry.each([&](auto m_entity)
			{
				Entity Entity(this, m_entity);
				if (Entity.GetComponent<StaticMeshComponent>().isFoliage == false)
				{
					Renderer3D::BeginScene(editor_cam);
					auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
					glm::vec4 color;

					auto mesh = Entity.GetComponent<StaticMeshComponent>();
					if (Entity.HasComponent<SpriteRenderer>()) {
						auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
						Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color, SpriteRendererComponent.m_Roughness, SpriteRendererComponent.m_Metallic);
					}
					else
						Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor); // default color, roughness, metallic value
				}
				else
				{
					Renderer3D::BeginSceneFoliage(editor_cam);
					auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
					glm::vec4 color;

					auto mesh = Entity.GetComponent<StaticMeshComponent>();
					if (Entity.HasComponent<SpriteRenderer>()) {
						auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
						Renderer3D::DrawFoliage(*mesh, transform, SpriteRendererComponent.Color, SpriteRendererComponent.m_Roughness, SpriteRendererComponent.m_Metallic);
					}
					else
						Renderer3D::DrawFoliage(*mesh, transform, Entity.m_DefaultColor); // default color, roughness, metallic value
				}
			});
			Renderer3D::EndScene();

			//Renderer3D::BeginScene(editor_cam);
			//Renderer3D::DrawMesh(*Plane, { 0,0,0 }, { 100,100,100 }, { 90,0,0 });

			Renderer3D::RenderShadows(*this, editor_cam);//shadows should be computed at last
			Renderer3D::AmbiantOcclusion(*this, editor_cam);
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
				m_registry.get<CameraComponent>(entity).camera.SetViewportSize(Width,Height);
		}
		editor_cam.SetViewportSize(Width, Height);//resize the editor camera
	}
	void Scene::OnEvent(Event& e)
	{
		editor_cam.OnEvent(e);
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