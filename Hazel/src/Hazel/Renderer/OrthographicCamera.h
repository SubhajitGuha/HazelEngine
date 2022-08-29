#pragma once
#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
namespace Hazel {
	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetPosition(const glm::vec3& position) { 
		m_Position = position;
		ReCalculateViewMatrix();
		}

		void SetRotation(float rotation) {
		m_Rotation = rotation;
		ReCalculateViewMatrix();
		}

		inline glm::vec3& GetPosition() { return m_Position; }
		inline float& GetRotation() { return m_Rotation; }
		inline glm::mat4 GetProjectionViewMatix() { return m_ProjectionView; }

		void ReCalculateViewMatrix();
	private:
		glm::mat4 m_View ;
		glm::mat4 m_Projection;
		glm::mat4 m_ProjectionView;

		glm::vec3 m_Position = {0,0,0};
		float m_Rotation;
	};
}