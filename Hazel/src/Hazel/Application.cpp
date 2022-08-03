#include "hzpch.h"
#include "Application.h"
#include "Log.h"
#include "Events/ApplicationEvent.h"

namespace Hazel {
	Application::Application()
	{
	}
	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent re(1920, 1080);
		HAZEL_WARN(re);
		
		while (true);
	}
}