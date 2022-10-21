#pragma once
#include "entt.hpp"
#include "Hazel/Core.h"

namespace Hazel {
	class Scene
	{
	public:
		Scene();
		~Scene();
		entt::registry& getRegistry() { return m_registry; }
		entt::entity CreateEntity();
		//const entt::registry& GetRegistry() { return m_registry; }

		static ref<Scene> Create();
	private:
		entt::registry m_registry;
	};
}

