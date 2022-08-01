#pragma once
#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();
int main(int *argc,char** argv) {
	
	//Hazel::Log l;
	Hazel::Log::init();
	HAZEL_CORE_ERROR("Hello");
	HAZEL_WARN("ERRROR!!!!!!!!!!!");
	auto app = Hazel::CreateApplication();
	app->Run();

	delete app;
}
#endif // HZ_PLATFORM_WINDOWS