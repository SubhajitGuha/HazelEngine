#include <Hazel.h>
#include <Hazel/EntryPoint.h>
#include "HazelEditor.h"

//using namespace Hazel;

class Sandbox :public Hazel::Application
{
public:
	Sandbox(){
		//PushLayer(new GameLayer());
		//PushOverlay(new Hazel::ImGuiLayer());
		PushLayer(new HazelEditor());
	}
	~Sandbox(){}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
