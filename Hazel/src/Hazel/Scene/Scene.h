#pragma once
#include "entt.hpp"
#include "Hazel/Log.h"
#include"Hazel/Renderer/FrameBuffer.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/TimeSteps.h"
#include "Hazel/Core.h"

namespace Hazel {
	class Entity;
	class LoadMesh;
	class PointLight;
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
		void AddPointLight(PointLight* light);
		static ref<Scene> Create();

	public:
		std::vector<PointLight*> m_PointLights;
		static LoadMesh* Sphere, *Cube , *Plane,*plant,*House,* Windmill ,*Fern;
		static unsigned int m_Scene_tex_id;
		static unsigned int m_Scene_depth_id;
		ref<FrameBuffer> framebuffer;
	private:
		
		entt::registry m_registry;
		entt::entity m_entity{entt::null};
		std::thread shadow_thread;
		//friend class Entity;

	};
}

