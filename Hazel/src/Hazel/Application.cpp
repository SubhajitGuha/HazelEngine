#include "hzpch.h"
#include "Application.h"
#include "Log.h"
#include"platform/WindowsInput.h"
#include "HazelCodes.h"
#include "Renderer/Renderer.h"
#include"Layer.h"

#define HZ_BIND_FN(x) std::bind(&Application::x,this,std::placeholders::_1)
/*
std::bind returns a function pointer that can be used as an argument in SetCallBackEvent(), also we are using std::bind
to pass the 'this' pointer as an argument of onEvent while also retriving a function pointer,
with out std::bind it is not possible to call OnEvent with an argument while also retreiving a function pointer
*/

namespace Hazel {
	
	Application* Application::getApplication;
	Application::Application()
	{
		getApplication = this;
		m_window = std::unique_ptr<Window>(Window::Create());
		m_window->SetCallbackEvent(HZ_BIND_FN(OnEvent));
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispach(e);
		dispach.Dispatch<WindowCloseEvent>(HZ_BIND_FN(closeWindow));

		HAZEL_CORE_TRACE(e);
		for (auto it = m_layerstack.end(); it != m_layerstack.begin();)
		{
			(*--it)->OnEvent(e);	//decrement the iterator here or it will lead to a resticted memory address
			if (e.m_Handeled)
				break;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_layerstack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* Overlay)
	{
		m_layerstack.PushOverlay(Overlay);
		Overlay->OnAttach();
	}

	bool Application::closeWindow(WindowCloseEvent& EventClose)
	{
		m_Running = false;
		return true;
	}

	void Application::Run()
	{
		while (m_Running) {
			
			m_window->OnUpdate();

			//layers
			for (Layer* layer : m_layerstack)
				layer->OnUpdate();

			//for ImguiLayers
			m_ImGuiLayer->Begin();

			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
	
			m_ImGuiLayer->End();

			if (Input::IsKeyPressed(HZ_KEY_1))
				HAZEL_CORE_TRACE("KeyPressed is ",HZ_KEY_1);
			
			HAZEL_CORE_TRACE(Input::GetCursorPosition().first);
			HAZEL_CORE_TRACE(Input::GetCursorPosition().second);
		}
	}

	
}