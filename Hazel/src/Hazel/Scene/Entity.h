#pragma once
#include "Scene.h"
namespace Hazel {
	class Entity
	{
	public:
		Entity(Scene& scene,entt::entity& entity);
		~Entity();
		template<typename T,typename ...Args>
		void AddComponent(Args&& ...arg) {
			scene.getRegistry().emplace<T>(entity, std::forward<Args...>(arg...));//forward all the arguments so use "..."
		}
		template <typename ... Args>
		void RemoveComponent() {
			scene.getRegistry().remove<Args...>(entity);
		}
		template<typename T>
		T& GetComponent() {
			return scene.getRegistry().get<T>(entity);
		}
	//	bool hasComponent();
	private:
		entt::entity& entity;
		Scene& scene;
	};
}