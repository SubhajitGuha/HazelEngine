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
		void onResize(float width, float height);
		glm::vec3 GetPosition() { return v3; }
		void SetCameraPosition(const glm::vec3& pos) { v3 = pos; }
		bool bCanBeRotated(bool val) {
			bCanRotate = val;
			return val;
		}
		inline void SetCameraSpeed(const float& val) { m_movespeed = val; }
	private:
		bool ZoomEvent(MouseScrollEvent& e);
		bool WindowResize(WindowResizeEvent& e);
	private:
		float m_ZoomLevel = 3.0f;
		float m_AspectRatio;
		OrthographicCamera m_Camera;

		glm::vec3 v3 = { 0,0,0 };

		float m_movespeed = 100;
		float r = 0;
		bool bCanRotate = true;
	};
}