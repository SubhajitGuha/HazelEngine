#include "hzpch.h"
#include "WindowsWindow.h"
#include "Hazel/\Log.h"

namespace Hazel {
	WindowsWindow::~WindowsWindow()
	{
		ShutDown();
	}
	void WindowsWindow::OnUpdate()
	{
	}
	void WindowsWindow::SetVsync(bool enable)
	{
		if (enable)
			glfwSwapInterval(1);
	}
	bool WindowsWindow::b_Vsync() const
	{
		return false;
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
		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_Data);
		SetVsync(true);
	}
	void WindowsWindow::ShutDown()
	{
		glfwDestroyWindow(m_window);
	}
}
