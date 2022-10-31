#pragma once
#include "glm/glm.hpp"
#include "Hazel/Renderer/Camareas/SceneCamera.h"

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
}