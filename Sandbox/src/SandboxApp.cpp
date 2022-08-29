#include <Hazel.h>

class Layerimplement :public Hazel::Layer {
public:
	Layerimplement()
		:Layer("Hazel_Layer")
	{}
	virtual void OnUpdate() {
		HAZEL_INFO("Layer Update!!");
	}

	void OnImGuiRender() {
		ImGui::Begin("test");
		//ImGui::imvec
		ImVec4 color = { 0.5, 0.9, 0.1, 1.0 };
		float v[3];
		ImGui::SliderFloat3("Slider", v, 0, 1);
		ImGui::Text("Hello");
		ImGui::End();
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
