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
		EntityName = name;
		m_entity = m_registry.create();
		auto entity = new Entity( this,m_entity);
		Entity_Map[EntityName] = entity;
		return entity;
	}
	void Scene::OnUpdate(TimeStep ts)
	{
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
		auto view = m_registry.view<TransformComponent>();
		for (auto entt : view) {
			auto& transform = m_registry.get<TransformComponent>(entt);
			Renderer2D::DrawQuad(transform, { 0,0.7,0.9,1 });
		}
			Renderer2D::EndScene();
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