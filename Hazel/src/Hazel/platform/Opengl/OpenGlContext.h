#pragma once
#include "Hazel/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"
#include "Hazel/Log.h"
#include "Hazel/Core.h"

namespace Hazel {
	class OpenGlContext :public GraphicsContext
	{
	public:
		OpenGlContext(ref<GLFWwindow>);
		void Init()override;
		void SwapBuffers()override;
	private:
		ref<GLFWwindow> m_GlfwWindow;
	};
}

