#pragma once
#include "glm/glm.hpp"

//this is a camera class for camera component so it has no transform , it only has projection matrix
namespace Hazel {
	class Camera {
	public:
		Camera() = default;
		Camera(glm::mat4 Proj)
			:m_projection(Proj) {}
		virtual ~Camera() = default;
		void SetProjection(const glm::mat4&);
		auto GetProjection() { return m_projection; }
	public:
		bool bIsMainCamera = true;
		bool IsResiziable = true;
	protected:
		glm::mat4 m_projection;
	};
}