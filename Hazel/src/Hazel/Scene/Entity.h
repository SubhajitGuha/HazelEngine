#pragma once
#include "Hazel.h"
#include "Scene.h"
namespace Hazel {
	class Entity
	{
	public:
		Entity(Scene* scene,const entt::entity& entity);
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
		entt::entity GetEntity() { return entity; }
	
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