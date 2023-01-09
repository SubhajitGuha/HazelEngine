#include <Hazel.h>

#include <Hazel/EntryPoint.h>
#include "Sandbox2dApp.h"
using namespace Hazel;

class Sandbox :public Hazel::Application
{
public:
	Sandbox(){
		//PushLayer(new GameLayer());
		//PushOverlay(new Hazel::ImGuiLayer());
		PushLayer(new SandBox2dApp());
	}
	~Sandbox(){}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
