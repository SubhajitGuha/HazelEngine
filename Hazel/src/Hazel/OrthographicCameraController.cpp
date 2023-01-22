#include "hzpch.h"
#include "OrthographicCameraController.h"
#include "Hazel/Input.h";
#include "Hazel/HazelCodes.h"

#define HZ_BIND_FN(x) std::bind(&OrthographicCameraController::x,this,std::placeholders::_1)

namespace Hazel {
	OrthographicCameraController::OrthographicCameraController(float aspectratio)
		:m_AspectRatio(aspectratio), m_Camera(-m_AspectRatio*m_ZoomLevel, m_AspectRatio* m_ZoomLevel,-m_ZoomLevel,m_ZoomLevel)
	{
	}
	void OrthographicCameraController::OnUpdate(TimeStep deltatime)
	{
		if (Input::IsKeyPressed(HZ_KEY_W))
			v3.y -= m_movespeed * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_S))
			v3.y += m_movespeed * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_A))
			v3.x -= m_movespeed * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_D))
			v3.x += m_movespeed * deltatime;

		if (bCanRotate)
		{
			if (Input::IsKeyPressed(HZ_KEY_E))
				r += 60 * deltatime;
			if (Input::IsKeyPressed(HZ_KEY_Q))
				r -= 60 * deltatime;
		}

		m_Camera.SetPosition(v3);
		m_Camera.SetRotation(r);
	}
	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrollEvent>(HZ_BIND_FN(ZoomEvent));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_FN(WindowResize));
	}
	void OrthographicCameraController::onResize(float width, float height)
	{
		m_AspectRatio = width/ height;
		m_Camera.SetOrthographicProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
	bool OrthographicCameraController::ZoomEvent(MouseScrollEvent& e)
	{
		m_ZoomLevel = std::clamp(m_ZoomLevel - e.GetYOffset(), 1.0f, 30.0f);
		
		m_Camera.SetOrthographicProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
	bool OrthographicCameraController::WindowResize(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() /(float) e.GetHeight();
		m_Camera.SetOrthographicProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
}
