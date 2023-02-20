#pragma once
#include "Hazel.h"
#include "entt.hpp"
#include "Component.h"
#include "Scene.h"
namespace Hazel {
	class Entity
	{
	public:
		Entity(Scene* scene,const entt::entity& entity);
		Entity() = default;
		//Entity(const entt::entity& entity);
		~Entity() = default;
		template<typename T,typename ...Args>
		T& AddComponent(Args&& ...arg) {
			if (HasComponent<T>())//if the entity already exist then return that entity
				RemoveComponent<T>();

			scene->getRegistry().emplace<T>(entity, std::forward<Args>(arg)...);//forward all the arguments so use "..."
			return GetComponent<T>();
		}
		template <typename ... Args>
		void RemoveComponent() {
				if (HasComponent<Args...>() == false)//check whether all the components are valid or not Args.. passes all the component types one by one
					return;
			scene->getRegistry().remove<Args...>(entity);
		}
		template<typename T>
		T& GetComponent() {
			HZ_ASSERT(HasComponent<T>(),"Entity dosent contain this component")//if there is no component assertion will be called
			return scene->getRegistry().get<T>(entity);
		}
		template<typename ...Args>
		auto View() {
			return scene->getRegistry().view<Args...>();
		}

		template<typename t,typename ...Args>
		t& ReplaceComponent(Args&& ...args)
		{
			return scene->getRegistry().replace<t,Args...>(entity, std::forward<Args>(args)...);
		}

		template<typename T>
		bool HasComponent()
		{
			auto x= scene->getRegistry().try_get<T>(entity);
		
			if (x )
				return true;
			else
				return false;
		}

		entt::entity GetEntity() { return entity; }

		operator uint32_t() const{ return (uint32_t)entity; }
		
		operator entt::entity()const { return entity; }

		bool operator== (uint32_t id) {
			return (uint32_t)entity == id;
		}
		
		bool operator!= (uint32_t id) {
			return !(*this == id);
		}

	//default entity parameters like color of the quad,texture,subtexture,transform,shader
	public:
		glm::vec4 m_DefaultColor = glm::vec4(1.0f);
	private:
		entt::entity entity{entt::null};
		Scene* scene;
	};
}