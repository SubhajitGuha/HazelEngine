#include "hzpch.h"
#include "SceneCamera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Hazel {
	SceneCamera::SceneCamera(float Width, float Height)
	{
		SetViewportSize(Width, Height);
	}
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}
	void SceneCamera::SetViewportSize(float width, float Height)
	{
		m_AspectRatio = width / Height;
		RecalculateProjection();
	}
	void SceneCamera::SetPerspective(float verticalFOV, float Near, float Far)
	{
		m_projection_type = ProjectionTypes::perspective;
		m_verticalFOV = verticalFOV;
		m_PerspectiveNear = Near;
		m_PerspectiveFar = Far;
		RecalculateProjection();
	}
	void SceneCamera::SetOrthographic(float Size, float Near, float Far)
	{
		m_projection_type = ProjectionTypes::Orhtographic;
		m_OrthographicSize = Size;
		m_OrthographicNear = Near;
		m_OrthographicFar = Far;
		RecalculateProjection();
	}
	void SceneCamera::RecalculateProjection()
	{
		switch (m_projection_type)
		{
		case ProjectionTypes::Orhtographic:
		{
			float left = -m_AspectRatio * m_OrthographicSize * 0.5;
			float right = m_AspectRatio * m_OrthographicSize * 0.5;
			float bottom = -m_OrthographicSize * 0.5;
			float Up = m_OrthographicSize * 0.5;
			m_projection = glm::ortho(left, right, bottom, Up, m_OrthographicNear, m_OrthographicFar);
			return;
		}
		case ProjectionTypes::perspective:
			m_projection = glm::perspective(glm::radians(m_verticalFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
			return;
		}
	}
}
