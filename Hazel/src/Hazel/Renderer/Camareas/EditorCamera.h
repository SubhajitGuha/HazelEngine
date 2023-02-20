#pragma once
#include "Camera.h"
#include "Hazel.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel {
	class EditorCamera :public Camera
	{
	public:
		EditorCamera();
		EditorCamera(float width, float Height);
		~EditorCamera() = default;

		void SetPerspctive(float v_FOV,float Near,float Far);
		glm::mat4 GetProjectionView() { return m_ProjectionView; }
		void OnEvent(Event& e);
		void OnUpdate(TimeStep ts);
		void SetViewportSize(float width,float Height);

	private:
		void RecalculateProjection();
		void RecalculateProjectionView();

	private:
		glm::mat4 m_View;
		glm::mat4 m_Projection;
		glm::mat4 m_ProjectionView;

		glm::vec3 m_Position = { 0,0,0 }, m_ViewDirection = { 0,0,1 };
		//m_Viewdirection is the location we are looking at (it is the vector multiplied with rotation matrix)
		glm::vec3 Up = { 0,1,0 } , RightVector;//we get right vector by getting the cross product of m_ViewDirection and Up vectors

		float m_verticalFOV = 45.0f;
		float m_PerspectiveNear = 0.01;
		float m_PerspectiveFar = 1000;
		
		float m_AspectRatio = 1.0;

		//camera parameters
		float m_movespeed = 0.02;
		glm::vec2 OldMousePos = { 0,0};
	};
}