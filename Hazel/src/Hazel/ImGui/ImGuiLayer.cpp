#include"hzpch.h"
#include"Hazel/Application.h"
#include"ImGuiLayer.h"
#include "backends/imgui_impl_opengl3.cpp"
#include "backends/imgui_impl_glfw.cpp"

#include"GLFW/glfw3.h"
#include "Hazel/Application.h"

#define BIND_FUNC(x) std::bind(&ImGuiLayer::x,this,std::placeholders::_1)

namespace Hazel {
	ImFont* ImGuiLayer::Font;
	ImGuiLayer::ImGuiLayer()
		:Layer("ImGuiLayer")
	{
		Font = nullptr;
	}
	ImGuiLayer::~ImGuiLayer()
	{

	}
	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Font/OpenSans-Bold.ttf", 18.0f);
		Font = io.Fonts->AddFontFromFileTTF("Assets/Font/OpenSans-ExtraBold.ttf", 20.0f);


		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		
		SetDarkThemeColors();

		GLFWwindow* window =(GLFWwindow*) Application::Get().GetWindow().GetNativeWindow();
		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);

		ImGui_ImplOpenGL3_Init("#version 410");

	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		//bool demo_window = false;
		//ImGui::ShowDemoWindow(&demo_window);
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		//ImGuiIO& io = ImGui::GetIO();
		//e.m_Handeled |= e.IsInCategory(EventCategory::EventCategoryMouse) & io.WantCaptureMouse;
		//e.m_Handeled |= e.IsInCategory(EventCategory::EventCategoryKeyboard) & io.WantCaptureKeyboard;		
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = { 0.1,0.105,0.1,1.0f };

		colors[ImGuiCol_Header] = { 0.2,0.205,0.21,1.0f };
		colors[ImGuiCol_HeaderHovered] = { 0.3,0.305,0.3,1.0f };
		colors[ImGuiCol_HeaderActive] = { 0.1,0.105,0.1,1.0f };

		colors[ImGuiCol_Button] = { 0.2,0.205,0.21,1.0f };
		colors[ImGuiCol_ButtonHovered] = { 0.4,0.405,0.4,1.0f };
		colors[ImGuiCol_ButtonActive] = { 0.1,0.105,0.1,1.0f };

		colors[ImGuiCol_FrameBg] = { 0.2,0.205,0.21,1.0f };
		colors[ImGuiCol_FrameBgHovered] = { 0.3,0.305,0.3,1.0f };
		colors[ImGuiCol_FrameBgActive] = { 0.1,0.105,0.1,1.0f };

		colors[ImGuiCol_Tab] = { 0.25,0.2505,0.251,1.0f };
		colors[ImGuiCol_TabHovered] = { 0.78,0.7805,0.78,1.0f };
		colors[ImGuiCol_TabActive] = { 0.681,0.6805,0.681,1.0f };
		colors[ImGuiCol_TabUnfocused] = { 0.35,0.3505,0.351,1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = { 0.5,0.505,0.51,1.0f };

		colors[ImGuiCol_TitleBg] = { 0.15,0.1505,0.15,1.0f };
		colors[ImGuiCol_TitleBgActive] = { 0.15,0.1505,0.15,1.0f };
		colors[ImGuiCol_TitleBgCollapsed]= { 0.95,0.1505,0.951,1.0f };
	}


}