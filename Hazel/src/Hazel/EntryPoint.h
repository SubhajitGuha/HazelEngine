#pragma once
#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();
int main(int *argc,char** argv) {
	
	//Hazel::Log l;
	Hazel::Log::init();
	HZ_PROFILE_BEGIN("Start_Profiling", "Benchmark/Start_Profiling.json");
	auto app = Hazel::CreateApplication();
	HZ_PROFILE_END();

	HZ_PROFILE_BEGIN("Running_Profiling", "Benchmark/Running_Profiling.json");
	app->Run();
	HZ_PROFILE_END();

	HZ_PROFILE_BEGIN("End_Profiling", "Benchmark/End_Profiling.json");
	delete app;
	HZ_PROFILE_END();

	return 0;
}
#endif // HZ_PLATFORM_WINDOWS