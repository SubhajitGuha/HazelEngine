#include "hzpch.h"
#include "OpenGlContext.h"
#include "glad/glad.h"

Hazel::OpenGlContext::OpenGlContext(GLFWwindow* window)
	:m_GlfwWindow(window)
{
}

void Hazel::OpenGlContext::Init()
{
	glfwMakeContextCurrent(m_GlfwWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		HAZEL_CORE_ERROR("Error in initilizing glad");
	}
}

void Hazel::OpenGlContext::SwapBuffers()
{
	glfwSwapBuffers(m_GlfwWindow);
}
