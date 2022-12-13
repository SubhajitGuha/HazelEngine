#pragma once
#include "Hazel.h"
#include "glm/glm.hpp"
#include "Hazel/Renderer/Camareas/SceneCamera.h"
#include "ScriptableEntity.h"

namespace Hazel {
	struct TransformComponent {
		glm::mat4 Transform;
		TransformComponent() = default;
		TransformComponent(const glm::mat4& transform)
			:Transform(transform){}
		operator glm::mat4& () { return Transform; }
	};

	struct CameraComponent {
		SceneCamera camera;
		CameraComponent() 
			:camera() 
		{}
		CameraComponent(float width,float height)
			:camera(width,height)
		{}
		operator Camera& () { return camera; }
	};

	struct ScriptComponent {
		ScriptableEntity* m_Script = nullptr;
		std::function<void()> CreateInstance;
		std::function<void()> DeleteInstance;

		//call bind function at the time of attaching the component and pass the inherited custom class of ScriptableEntity as template
		template<typename t>
		void Bind()
		{
			CreateInstance = [&]() {m_Script = new t(); };
			DeleteInstance = [&]() {delete m_Script; };
		}
	};
}