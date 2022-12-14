#pragma once
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"
#include "Hazel/Core/TimeSteps.h"

namespace Hazel {
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectratio);
		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);
		inline OrthographicCamera GetCamera() { return m_Camera; }
	private:
		bool ZoomEvent(MouseScrollEvent& e);
		bool WindowResize(WindowResizeEvent& e);
	private:
		float m_ZoomLevel = 1.0f;
		float m_AspectRatio;
		OrthographicCamera m_Camera;

		glm::vec3 v3 = { 0,0,0 };

		float m_movespeed = 20;
		float r = 0;
	};
}