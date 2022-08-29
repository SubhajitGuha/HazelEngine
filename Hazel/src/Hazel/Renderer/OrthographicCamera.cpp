#include "hzpch.h"
#include "OrthographicCamera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Hazel {
	OrthographicCamera::OrthographicCamera(float left,float right,float bottom,float top)
		:m_Projection(glm::ortho<float>(left,right,bottom,top,-1.0,1.0)),m_View(1.0)
	{
		m_ProjectionView = m_Projection * m_View;
	}
	void OrthographicCamera::ReCalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * 
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0, 0.0, 1.0));

		m_View = glm::inverse(transform); // invert the transform matrix to get the OrthographicCamera effect(i.e when OrthographicCamera moves up the object moves down and so on)
		m_ProjectionView = m_Projection * m_View;
	}
}