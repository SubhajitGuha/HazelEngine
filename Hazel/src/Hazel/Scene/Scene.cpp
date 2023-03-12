#include "hzpch.h"
#include "Scene.h"
#include "Entity.h"
#include "Component.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/Camareas/EditorCamera.h"
#include "glad/glad.h"
#include "Hazel/LoadMesh.h"

namespace Hazel {
	
	 LoadMesh* Scene::m_LoadMesh=nullptr, *Scene::Cube= nullptr, *Scene::Plane= nullptr;
	EditorCamera editor_cam;
	bool capture = false;
	Scene::Scene()
	{
		m_LoadMesh = new LoadMesh("Assets/Meshes/Sphere.obj");
		Cube = new LoadMesh("Assets/Meshes/Cube.obj");
		Plane = new LoadMesh("Assets/Meshes/Plane.obj");
		editor_cam.SetViewportSize(1920.0,1080.0);
	}
	Scene::~Scene()
	{
	}
	Entity* Scene::CreateEntity(const std::string& name)
	{
		m_entity = m_registry.create();
		auto entity = new Entity( this,m_entity);
		entity->AddComponent<TransformComponent>();
		if (name == "")//if no name is give to an entity just call it entity (i.e define tag with entity)
			entity->AddComponent<TagComponent>("Entity");//automatically add a tag component when an entity is created
		else
			entity->AddComponent<TagComponent>(name);
		//Entity_Map[entity->GetComponent<TagComponent>()] = entity;
		Renderer3D::SetUpCubeMapReflections(*this);
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

		Camera* MainCamera=nullptr;//if there is no main camera Then dont render
		{
			auto view = m_registry.view<CameraComponent>();
			for (auto entt : view) {
				auto& camera = m_registry.get<CameraComponent>(entt);
				if (camera.camera.bIsMainCamera)
					MainCamera = &camera.camera;
			}
		}

		if (!MainCamera)
			Renderer3D::BeginScene(editor_cam);
		else
			Renderer3D::BeginScene(*MainCamera);//pass only the main camera for rendering

		m_registry.each([&](auto m_entity)
			{
				//auto entt = item.second->GetEntity();//get the original entity (i.e. entt::entity returns an unsigned int)
				Entity Entity(this, m_entity);
				auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
				glm::vec4 color;

				//if (camera.camera.bIsMainCamera) {
				if (Entity.HasComponent<SpriteRenderer>()) {
					auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
					Renderer3D::DrawMesh(*m_LoadMesh, transform, SpriteRendererComponent.Color);
					//Renderer2D::DrawQuad(transform, SpriteRendererComponent.Color, SpriteRendererComponent.texture);
				}
				else
					Renderer3D::DrawMesh(*Cube, transform, Entity.m_DefaultColor);
					//Renderer2D::DrawQuad(transform, Entity.m_DefaultColor,nullptr);//running the script
				
			});
			Renderer3D::EndScene();

			Renderer3D::BeginScene(editor_cam);
			Renderer3D::DrawMesh(*Plane, { 0,0,0 }, { 100,100,100 }, { 0,0,0 });
			//Renderer3D::EndScene();
			
			Renderer3D::SetUpCubeMapReflections(*this);

			Renderer3D::RenderShadows(*this, editor_cam);

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
	ref<Scene> Scene::Create()
	{
		return std::make_shared<Scene>();
	}

}