#include "hzpch.h"
#include "Scene.h"

namespace Hazel {
	Scene::Scene()
	{
	}
	Scene::~Scene()
	{
	}
	entt::entity Scene::CreateEntity()
	{
		return m_registry.create();//creates a blank entity
	}
	ref<Scene> Scene::Create()
	{
		return std::make_shared<Scene>();
	}

}
