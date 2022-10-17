#pragma once
#include "entt.hpp"
#include "Hazel/Core.h"

namespace Hazel {
	class Scene
	{
	public:
		Scene();
		~Scene();
		entt::entity CreateEntity();
		//entt::registry GetRegistry() { return m_registry; }
		//template<typename T>
		//void AddComponent(const entt::entity entity){m_registry.emplace<T>() }

		static ref<Scene> Create();
	public:
		entt::registry m_registry;
	};
}

