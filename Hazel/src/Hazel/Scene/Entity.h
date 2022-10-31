#pragma once
#include "Scene.h"
namespace Hazel {
	class Entity
	{
	public:
		Entity(Scene* scene,const entt::entity& entity);
		//Entity(const entt::entity& entity);
		~Entity() = default;
		template<typename T,typename ...Args>
		void AddComponent(Args&& ...arg) {
			scene->getRegistry().emplace<T>(entity, std::forward<Args>(arg)...);//forward all the arguments so use "..."
		}
		template <typename ... Args>
		void RemoveComponent() {
			scene->getRegistry().remove<Args...>(entity);
		}
		template<typename T>
		T& GetComponent() {
			return scene->getRegistry().get<T>(entity);
		}
		template<typename ...Args>
		auto View() {
			return scene->getRegistry().view<Args...>();
		}
	
	//	bool hasComponent();
	private:
		entt::entity entity{entt::null};
		Scene* scene;
	};
}