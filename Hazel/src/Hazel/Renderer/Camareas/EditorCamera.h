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
		inline glm::mat4 GetViewMatrix() { return m_View; }
		inline glm::mat4 GetProjectionMatrix() { return m_Projection; }
		void OnEvent(Event& e);
		void OnUpdate(TimeStep ts);
		void SetViewportSize(float width,float Height);
		inline glm::vec3 GetCameraPosition() { return m_Position; }

		void SetCameraPosition(const glm::vec3& pos) { m_Position = pos; RecalculateProjectionView();}
		void SetViewDirection(const glm::vec3& dir) { m_ViewDirection = dir; RecalculateProjectionView(); }
		void SetUPVector(const glm::vec3& up) {	Up = up; RecalculateProjectionView();}
		void RotateCamera(float yaw, float pitch);
		inline glm::vec3 GetViewDirection() { return m_ViewDirection; }
		inline float GetAspectRatio() { return m_AspectRatio; }
		inline float GetVerticalFOV() { return m_verticalFOV; }

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

		float m_verticalFOV = 90.0f;
		float m_PerspectiveNear = 0.01;
		float m_PerspectiveFar = 1000;
		
		float m_AspectRatio = 1.0;

		//camera parameters
		float m_movespeed = 0.2;
		glm::vec2 OldMousePos = { 0,0};
	};
}