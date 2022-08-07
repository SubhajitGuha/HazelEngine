#pragma once
#include "hzpch.h"
#include "Core.h"
#include"Window.h"
#include "Events/ApplicationEvent.h"

namespace Hazel {

	class HAZEL_API Application { //set this class as dll export
	public:
		Application();

		virtual ~Application();
		
		void Run();

		void OnEvent(Event& e);
		std::unique_ptr<Window> m_window;

	private:
		bool m_Running = true;
		bool closeWindow(WindowCloseEvent&);
	};
	//define in client (not in engine dll)
	Application* CreateApplication();
	
}