#include "hzpch.h"
#include "WindowsWindow.h"
#include "Hazel/Log.h"
#include"Hazel/Events/KeyEvent.h"
#include"Hazel/Events/ApplicationEvent.h"
#include"Hazel/Events/MouseEvent.h"
#include "glad/glad.h"


namespace Hazel {
	WindowsWindow::WindowsWindow()
	{
		Init(prop);
	}
	WindowsWindow::~WindowsWindow()
	{
		//delete m_window;
		//delete Context;
		ShutDown();
	}
	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		Context->SwapBuffers();
		
	}
	void WindowsWindow::SetVsync(bool enable)
	{
		if (enable)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
	}
	bool WindowsWindow::b_Vsync() const
	{
		return true;
	}
	void WindowsWindow::Init(WindowProp& prop)
	{
		m_Data.height = prop.height;
		m_Data.width = prop.width;
		m_Data.name = prop.Title;

		if (m_isInitilized == false) {
			
			if (!glfwInit())
			{
				HAZEL_CORE_ERROR("GLFW Not Initilized!!");
			}

			m_isInitilized = true;
		}

		m_window = glfwCreateWindow((int)m_Data.width,(int) m_Data.height, m_Data.name.c_str(), nullptr, nullptr);
		Context = new OpenGlContext((ref<GLFWwindow>)m_window);//context for opengl as this piece of code is abstracted away
		//glad is initilized in OpenGlContext->Init() class
		Context->Init();
		glfwSetWindowUserPointer(m_window, &m_Data);
		SetVsync(false);
		
		//set GLFW callbacks
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.width = width;
			data.height = height;
			WindowResizeEvent Eventresize(width, height);
			data.Callbackfunc(Eventresize);//this calls the OnEvent () func in application class
			});


		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS:
			{
				MouseButtonPressed EventPressed(button);
				data.Callbackfunc(EventPressed);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleased EventReleased(button);
				data.Callbackfunc(EventReleased);
				break;
			}
			}

			});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrollEvent EventScroll(xoffset, yoffset);
			data.Callbackfunc(EventScroll);
			});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMoveEvent EventCursorPos(xpos, ypos);
			data.Callbackfunc(EventCursorPos);
			});

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				KeyPressedEvent EventKeyPressed(key, 0);
				data.Callbackfunc(EventKeyPressed);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent EventKeyReleased(key);
				data.Callbackfunc(EventKeyReleased);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent EventKeyRepeat(key, 1);
				data.Callbackfunc(EventKeyRepeat);
				break;
			}
			}
			});

		glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int codepoint) {
			WindowData& Data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent Typed(codepoint);
			Data.Callbackfunc(Typed);
			});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
			WindowData& Data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent EventWindClose;
			Data.Callbackfunc(EventWindClose);
			});
	}
	void WindowsWindow::ShutDown()
	{
		glfwDestroyWindow(m_window);
	}
}
