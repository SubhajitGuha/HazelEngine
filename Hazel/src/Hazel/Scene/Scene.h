#pragma once
#include "entt.hpp"
#include "Hazel/Log.h"
#include "Hazel/Core/TimeSteps.h"
#include "Hazel/Core.h"
#include <unordered_map>

namespace Hazel {
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();
		entt::registry& getRegistry() { return m_registry; }
		Entity* CreateEntity(const std::string& name);
		//const entt::registry& GetRegistry() { return m_registry; }
		void OnUpdate(TimeStep ts);
		void OnCreate();
		void Resize(float Width, float Height);

		Entity* GetEntitybyName(const char* Name)
		{
			if (Entity_Map.find(Name) == Entity_Map.end()) {
				HAZEL_CORE_ERROR("No Such Entity With Name {0}", Name);
			}
				return Entity_Map[Name];
		}

		static ref<Scene> Create();
	public:
		std::string EntityName;
	private:
		entt::registry m_registry;
		entt::entity m_entity{entt::null};
		std::unordered_map<std::string, Entity*> Entity_Map;
		Entity* m_Entity;
	};
}

