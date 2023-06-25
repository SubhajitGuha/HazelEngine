#pragma once
#include "entt.hpp"
#include "Hazel/Log.h"
#include"Hazel/Renderer/FrameBuffer.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/TimeSteps.h"
#include "Hazel/Renderer/Camareas/Camera.h"
#include "Hazel/Core.h"

namespace Hazel {
	class Entity;
	class LoadMesh;
	class ScriptableEntity;
	class EditorCamera;
	class Camera;
	class PointLight;
	class Bloom;
	class Scene
	{
	public:
		static bool TOGGLE_SHADOWS;//universal switch to enable or disable shadows
		static bool TOGGLE_SSAO;////universal switch to enable or disable ssao
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
		void PostProcess();
	public:
		std::vector<PointLight*> m_PointLights;
		static LoadMesh* Sphere, *Sphere_simple, *Cube , *Plane,*plant,*House,* Windmill ,*Fern, *Sponza;
		static unsigned int m_Scene_tex_id;
		static unsigned int m_Scene_depth_id;
		ref<FrameBuffer> framebuffer;
		std::unordered_map<size_t, ScriptableEntity*> m_scriptsMap;
		static float foliage_dist, num_foliage;
		Camera* MainCamera = nullptr;//if there is no main camera Then dont render
		ref<Bloom> m_Bloom;

	private:
		entt::registry m_registry;
		entt::entity m_entity{entt::null};
		std::thread shadow_thread;
		//friend class Entity;

	};
}

