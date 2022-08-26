#include "hzpch.h"
#include "Application.h"
#include "Log.h"
#include <glad/glad.h>
#include"platform/WindowsInput.h"
#include "HazelCodes.h"
#include "Renderer/Renderer.h"


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
		float pos[] = 
		{-0.8,0.8,0.0,  0.8 ,0.0 ,0.6 ,1.0,
		0.8,-0.8,0.0,   0.1 ,0.6 ,0.9 ,1.0,
		-0.8,-0.80,0.0, 0.8 ,0.7 ,0.0 ,1.0,
		0.8,0.8,0.0,    0.2 ,0.9 ,0.8 ,1.0,
		0.0,0.5,0.0,    0.3 ,0.3 ,0.3 ,1.0,
		0.5,-0.5,0.0,   0.3 ,0.3 ,0.3 ,1.0,
		-0.5,-0.5,0.0,   0.3 ,0.3 ,0.3 ,1.0};

		unsigned int index[] =
		{0,1,2,
		0,1,3,
		4,5,6};
		
		VertexArray* vao=VertexArray::Create();//vertex array
		VertexBuffer* vb = VertexBuffer::Create(pos, sizeof(pos));//vertex buffer
		BufferLayout bl;
		bl.push("position", DataType::Float3);
		bl.push("color", DataType::Float4);

		IndexBuffer* ib = IndexBuffer::Create(index, sizeof(index));

		vao->AddBuffer(bl, *vb);
		vao->SetIndexBuffer((std::shared_ptr<IndexBuffer>)ib);

		std::string vertexshader = R"(
			#version 410 core
			layout (location = 0) in vec3 pos;
			layout (location = 1) in vec4 col;

			out vec4 m_color;

			void main()
			{
				m_color = col;
				gl_Position = vec4(pos ,1.0);
				
			}
		)";
		std::string fragmentshader = R"(
			#version 410 core
			layout (location = 0) out vec4 color;

			in vec4 m_color;

			void main()
			{
				color= m_color;
			}
		)";
		shader = new Shader(vertexshader, fragmentshader);

		while (m_Running) {
			
			m_window->OnUpdate();
			
			RenderCommand::ClearColor(glm::vec4(0.8,0.8,0.8,0.8));
			RenderCommand::Clear();

			Renderer::BeginScene();
			Renderer::Submit(*vao);
			Renderer::EndScene();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_layerstack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			if (Input::IsKeyPressed(HZ_KEY_1))
				HAZEL_CORE_TRACE("KeyPressed is ",HZ_KEY_1);
			
			HAZEL_CORE_TRACE(Input::GetCursorPosition().first);
			HAZEL_CORE_TRACE(Input::GetCursorPosition().second);
		}
	}
}