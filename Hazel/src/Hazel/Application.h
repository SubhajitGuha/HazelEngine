#pragma once
#include "hzpch.h"
#include "Core.h"
#include"Window.h"
#include "Events/ApplicationEvent.h"
#include "LayerStack.h"
#include "ImGui/ImGuiLayer.h"
#include "Hazel/Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "HazelCodes.h"

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

		bool MoveForward(KeyPressedEvent& key) {
			if (key.GetKeyCode() == 17) {
				v += 0.05;
			}
			return true;
		}
	public:
		glm::vec3 v3;
		float v=0;
	private:
		ImGuiLayer* m_ImGuiLayer;
		std::unique_ptr<Window> m_window;
		bool m_Running = true;
		bool closeWindow(WindowCloseEvent&);
		LayerStack m_layerstack;
		static Application* getApplication;
		Shader* shader;
		OrthographicCamera m_camera;
		
	};
	//define in client (not in engine dll)
	Application* CreateApplication();
	
}