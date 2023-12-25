#include "hzpch.h"
#include "EditorCamera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Hazel/Renderer/CubeMapEnvironment.h"

namespace Hazel {
	EditorCamera::EditorCamera()
		:m_View(1.0)
	{
		RightVector = glm::normalize(glm::cross(m_ViewDirection, Up));
		m_Projection = glm::perspective(glm::radians(m_verticalFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		m_ProjectionView = m_Projection * m_View;
		m_movespeed = 50;
	}
	EditorCamera::EditorCamera(float width, float Height)
	{
		bIsMainCamera = true;
		RightVector = glm::cross(m_ViewDirection, Up);
		SetViewportSize(width/Height);
		m_movespeed = 50;

	}

	void EditorCamera::SetViewportSize(float aspectratio)
	{
		m_AspectRatio = aspectratio;
		RecalculateProjection();
		RecalculateProjectionView();
	}

	void EditorCamera::RotateCamera(float pitch, float yaw, float roll)
	{
		m_pitch = pitch;
		m_yaw = yaw;
		m_roll = roll;
		//glm::quat rotation = glm::angleAxis(glm::radians(yaw), Up) * glm::angleAxis(glm::radians(pitch),RightVector) ;
		//m_ViewDirection = glm::rotate(rotation, m_ViewDirection);
		m_ViewDirection = glm::mat3(glm::rotate(glm::radians(yaw), Up)) * glm::mat3(glm::rotate(glm::radians(pitch), RightVector)) * glm::mat3(glm::rotate(glm::radians(roll), m_ViewDirection)) * m_ViewDirection;
		//m_ViewDirection = rotation * glm::vec3(0, 0, 1);
		RecalculateProjectionView();
	}

	void EditorCamera::RecalculateProjection()
	{
		// ** Take Vertical Fov in radians
		m_Projection = glm::perspective(glm::radians(m_verticalFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
	}

	void EditorCamera::SetPerspctive(float v_FOV, float Near, float Far)
	{
		m_verticalFOV = v_FOV;
		m_PerspectiveNear = Near;
		m_PerspectiveFar = Far;

		RecalculateProjection();
		RecalculateProjectionView();
	}
	
	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dis(e);

		dis.Dispatch<MouseScrollEvent>([&](MouseScrollEvent& event) {
			m_verticalFOV += event.GetYOffset() * 0.1;//change the fov (idk how to implement zoom in camera so I change the FOV :) )
			RecalculateProjection();
			RecalculateProjectionView();
			return true; });
	}

	void EditorCamera::OnUpdate(TimeStep deltatime)
	{
		RightVector  = glm::cross(m_ViewDirection, Up);//we get the right vector (as it is always perpendicular to up and m_ViewDirection)
		
		if (Input::IsKeyPressed(HZ_KEY_W))
			m_Position += m_ViewDirection * glm::vec3(m_movespeed*deltatime);//move along the View direction
		if (Input::IsKeyPressed(HZ_KEY_S))
			m_Position -= m_ViewDirection * glm::vec3(m_movespeed*deltatime);//move along the View direction
		if (Input::IsKeyPressed(HZ_KEY_A))
			m_Position -= RightVector * glm::vec3(m_movespeed*deltatime);//move along the right vector
		if (Input::IsKeyPressed(HZ_KEY_D))
			m_Position += RightVector * glm::vec3(m_movespeed*deltatime);
		if(Input::IsKeyPressed(HZ_KEY_Q))
			m_Position += Up * glm::vec3(m_movespeed*deltatime);//move along up vector
		if (Input::IsKeyPressed(HZ_KEY_E))
			m_Position -= Up * glm::vec3(m_movespeed*deltatime);
		//if (Input::IsKeyPressed(HZ_KEY_R))//reset camera when R is pressed
		//{
		//	m_Position = { 0,0,-1 };
		//	m_ViewDirection = { 0,0,1 };
		//	m_verticalFOV = 45.0f;
		//}

		glm::vec2 NewMousePos = { Input::GetCursorPosition().first,Input::GetCursorPosition().second };
		if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_2))//camera pan
		{
			auto delta = NewMousePos - OldMousePos;//get change in mouse position

			m_ViewDirection = glm::mat3(glm::rotate(glm::radians(-delta.x) * 0.1f, Up)) * m_ViewDirection;//invert it
			m_ViewDirection = glm::mat3(glm::rotate(glm::radians(-delta.y) * 0.1f, RightVector)) * m_ViewDirection;//rotate along right vector
			m_ViewDirection = glm::normalize(m_ViewDirection);
		}
		OldMousePos = NewMousePos;

		RecalculateProjectionView();
	}

	void EditorCamera::RecalculateProjectionView()
	{
		//moving the camera is nothing but transforming the world
		//glm::vec3 up = glm::normalize(glm::cross(RightVector, glm::normalize(m_ViewDirection)));
		m_View = glm::lookAt(m_Position - glm::normalize(m_ViewDirection), m_Position, Up);//this gives the view matrix
		m_ProjectionView = m_Projection * m_View;
	}
}
