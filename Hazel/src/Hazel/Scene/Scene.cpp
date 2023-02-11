#include "hzpch.h"
#include "Scene.h"
#include "Entity.h"
#include "Component.h"
#include "Hazel/Renderer/Renderer2D.h"

namespace Hazel {
	Scene::Scene()
	{
	}
	Scene::~Scene()
	{
	}
	Entity* Scene::CreateEntity(const std::string& name)
	{
		m_entity = m_registry.create();
		auto entity = new Entity( this,m_entity);
		if (name == "")//if no name is give to an entity just call it entity (i.e define tag with entity)
			entity->AddComponent<TagComponent>("Entity");//automatically add a tag component when an entity is created
		else
			entity->AddComponent<TagComponent>(name);
		Entity_Map[entity->GetComponent<TagComponent>()] = entity;
		return entity;
	}
	void Scene::OnUpdate(TimeStep ts)
	{
		//run scripts
		m_registry.view<ScriptComponent>().each([=](entt::entity entity, ScriptComponent& nsc) 
		{
				if(nsc.m_Script==nullptr)
				nsc.CreateInstance();// this needs to be done once ,not every frame.
			
			for (auto item : Entity_Map)
			{
				if (entity == item.second->GetEntity())
				{
					nsc.m_Script->m_Entity = item.second;//the Entity in the script class is made equal to the created scene entity
					m_Entity = nsc.m_Script->m_Entity;//this allows more script flexibility and discards unnecessary memory allocations
				}
			}
			

			nsc.m_Script->OnUpdate(ts);//update to get the script values

			for (auto item : Entity_Map)
			{
				if (entity == item.second->GetEntity())
				{
					item.second->m_DefaultColor = m_Entity->m_DefaultColor;//then update the entity attributes
					//can be a shader,can be a model , ...... anything that can be changed
				}
			}

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
				return;

			Renderer2D::BeginScene(*MainCamera);//pass only the main camera for rendering

		for (auto item : Entity_Map) //iterate through all the Entity in the map Entity_map
		{
			auto entt = item.second->GetEntity();//get the original entity (i.e. entt::entity returns an unsigned int)

			auto& transform = item.second->GetComponent<TransformComponent>();
			auto& camera = item.second->GetComponent<CameraComponent>();
			glm::vec4 color;
			
			if (m_Entity && entt == m_Entity->GetEntity())//if the current entity has a script
				color = m_Entity->m_DefaultColor;
			else
				color = glm::vec4(1.0f);

			if (camera.camera.bIsMainCamera) {
				Renderer2D::DrawQuad(transform, item.second->m_DefaultColor);//running the script
			}
		}

			Renderer2D::EndScene();
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
	}
	ref<Scene> Scene::Create()
	{
		return std::make_shared<Scene>();
	}

}