#pragma once
#include "hzpch.h"
#include "Core.h"
#include"Window.h"
#include "Events/ApplicationEvent.h"
#include "LayerStack.h"

namespace Hazel {

	class HAZEL_API Application { //set this class as dll export
	public:
		Application();

		virtual ~Application();
		
		void Run();

		void OnEvent(Event& e);
		
		void PushLayer(Layer* layer);
		
		void PushOverlay(Layer* Overlay);

		inline Window& GetWindow() { return *m_window; }

		static inline Application& Get() { return *getApplication; }
	private:
		std::unique_ptr<Window> m_window;
		bool m_Running = true;
		bool closeWindow(WindowCloseEvent&);
		LayerStack m_layerstack;
		static Application* getApplication;
	};
	//define in client (not in engine dll)
	Application* CreateApplication();
	
}