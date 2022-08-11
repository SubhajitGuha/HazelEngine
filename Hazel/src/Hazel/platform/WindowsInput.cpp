#include"hzpch.h"
#include "WindowsInput.h"
#include "Hazel/Application.h"

namespace Hazel {
	Input* Input::m_Input = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keyCode)
	{
		
		int status = glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), keyCode);
		return status==GLFW_PRESS || status==GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressed(int Button)
	{
		
		int status = glfwGetMouseButton((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), Button);
		return status == GLFW_PRESS;
	}

}