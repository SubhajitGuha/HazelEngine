#include "hzpch.h"
#include "OpenGlRenderer.h"
#include "glad/glad.h"

Hazel::OpenGlRenderer::OpenGlRenderer(GLFWwindow* window)
	:m_GlfwWindow(window)
{
}

void Hazel::OpenGlRenderer::Init()
{
	glfwMakeContextCurrent(m_GlfwWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		HAZEL_CORE_ERROR("Error in initilizing glad");
	}
}

void Hazel::OpenGlRenderer::SwapBuffers()
{
	glfwSwapBuffers(m_GlfwWindow);
}
