#pragma once
#include "entt.hpp"
#include "Hazel/Log.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/TimeSteps.h"
#include "Hazel/Core.h"


namespace Hazel {
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();
		entt::registry& getRegistry() { return m_registry; }
		Entity* CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(const Entity& delete_entity);
		//const entt::registry& GetRegistry() { return m_registry; }
		void OnUpdate(TimeStep ts);
		void OnCreate();
		void Resize(float Width, float Height);
		void OnEvent(Event& e);
		static ref<Scene> Create();

	private:
		
		entt::registry m_registry;
		entt::entity m_entity{entt::null};
		friend class Entity;

	};
}

