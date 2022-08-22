#pragma once
#include "Hazel/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"
#include "Hazel/Log.h"

namespace Hazel {
	class OpenGlContext :public GraphicsContext
	{
	public:
		OpenGlContext(GLFWwindow*);
		void Init()override;
		void SwapBuffers()override;
	private:
		GLFWwindow* m_GlfwWindow;
	};
}

