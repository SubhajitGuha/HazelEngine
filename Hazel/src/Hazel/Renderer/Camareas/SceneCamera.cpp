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
	void SceneCamera::SetOrthographic(float Size, float Near, float Far)
	{
		m_OrthographicSize = Size;
		m_OrthographicNear = Near;
		m_OrthographicFar = Far;
		RecalculateProjection();
	}
	void SceneCamera::RecalculateProjection()
	{
		float left = -m_AspectRatio * m_OrthographicSize * 0.5;
		float right = m_AspectRatio * m_OrthographicSize * 0.5;
		float bottom = -m_OrthographicSize * 0.5;
		float Up = m_OrthographicSize * 0.5;
		m_projection = glm::ortho(left, right, bottom, Up, m_OrthographicNear, m_OrthographicFar);
	}
}
