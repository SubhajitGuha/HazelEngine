#include "hzpch.h"
#include "Application.h"
#include "Log.h"
#include"platform/WindowsInput.h"
#include "HazelCodes.h"
#include "Renderer/Renderer.h"
#include"Layer.h"
#include"Log.h"

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

		Renderer::Init();//initilize the scene , enable blending,get gpu info,set culling dist

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
		dispach.Dispatch<WindowResizeEvent>([](WindowResizeEvent e) {
			Renderer::WindowResize(e.GetWidth(), e.GetHeight());
			return false; });

		HAZEL_CORE_TRACE(e);
		for (auto it = m_layerstack.end(); it != m_layerstack.begin();)
		{
			if (e.m_Handeled)
				break;
			(*--it)->OnEvent(e);	//decrement the iterator here or it will lead to a resticted memory address
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
		while (m_Running) //render loop
		{
			
			m_window->OnUpdate();

			float time = glfwGetTime();
			TimeStep ts = time - m_LastFrameTime;//this is the delta time (time btn last and present frame or time required to render a frame)
			m_LastFrameTime = time;
			
			//layers render layer and game layers
			for (Layer* layer : m_layerstack)
				layer->OnUpdate(ts);

			//for ImguiLayers
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();
		}
	}

	
}