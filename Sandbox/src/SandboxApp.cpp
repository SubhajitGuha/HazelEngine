#include <Hazel.h>

class Layerimplement :public Hazel::Layer {
public:
	Layerimplement()
		:Layer("Hazel_Layer")
	{}
	virtual void OnUpdate() {
		HAZEL_INFO("Layer Update!!");
	}

	virtual void OnEvent(Hazel::Event& e) {
		HAZEL_TRACE(e);
	}
};

class Sandbox :public Hazel::Application
{
public:
	Sandbox(){
		PushLayer(new Layerimplement());
		//PushOverlay(new Hazel::ImGuiLayer());
	}
	~Sandbox(){}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
