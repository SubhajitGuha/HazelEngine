#pragma once
#include "Hazel.h"
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
			scene->getRegistry().emplace<T>(entity, std::forward<Args>(arg)...);//forward all the arguments so use "..."
			return GetComponent<T>();
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

		template<typename t,typename ...Args>
		t& ReplaceComponent(Args&& ...args)
		{
			return scene->getRegistry().replace<t,Args...>(entity, std::forward<Args>(args)...);
		}

		template<typename T>
		bool HasComponent()
		{
			
			T* component = &GetComponent<T>();
			if (component == nullptr)
				return false;
			else
				return true;
		}

		entt::entity GetEntity() { return entity; }

		operator uint32_t() { return (uint32_t)entity; }
		
		bool operator== (uint32_t id) {
			return (uint32_t)entity == id;
		}
		
	//default entity parameters like color of the quad,texture,subtexture,transform,shader
	public:
		glm::vec4 m_DefaultColor = glm::vec4(1.0f);
		//ref<Texture2D> m_DefaultTexture;//default texture
		//ref<SubTexture2D> m_DefaultSubTrexture;//default subtexture
		ref<Shader> m_DefaultShader = Shader::Create("Assets/Shaders/2_In_1Shader.glsl");//default shader
	private:
		entt::entity entity{entt::null};
		Scene* scene;
	};
}