#include "Sandbox2dApp.h"

//#include "Hazel/Profiling.h"

SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1920.0 / 1080.0)
{
	//HZ_PROFILE_SCOPE("SandBox2dApp::SandBox2dApp()");

	m_Framebuffer = FrameBuffer::Create({ 1920,1080 });
	Renderer2D::Init();
}

void SandBox2dApp::OnAttach()
{
	m_Points = { { 5,-5 },{8,-8},{10,0} ,{12,3},{15,2},{16,3},{18,2.1},{18.2,3.5},{19,3.9},{19.8,-5},{20.5,2} };//array of points for testing
}

void SandBox2dApp::OnDetach()
{
}

void SandBox2dApp::OnUpdate(float deltatime)
{
	m_Framebuffer->Bind();

	RenderCommand::ClearColor({ 0.1,0.1,0.1,1 });
	RenderCommand::Clear();

	m_camera.OnUpdate(deltatime);

	//auto r = Font::Create();

	auto velocity = [&](const glm::vec2& point1, const glm::vec2& point2, float& s) {
		glm::vec2 vel;
		vel = point2 - point1;
		vel.x *= s;
		vel.y *= s;
		return vel;
	};

	glm::vec2 tmp_point = P1, tmp_vel = velocity(P0, m_Points[0], factor);

	Renderer2D::LineBeginScene(m_camera.GetCamera());
	for (int i = 0; i < m_Points.size() - 1; i++)
	{
		auto x = velocity(tmp_point, m_Points[i + 1], factor);
		Renderer2D::DrawCurve(tmp_point, m_Points[i], tmp_vel, x);//drawing a hermitian curve
		tmp_point = m_Points[i];
		tmp_vel = x;
		Renderer2D::Draw_Bezier_Curve(P0, c1, c2, P1);
	}
	Renderer2D::LineEndScene();
	
	//Renderer2D::BeginScene(m_camera.GetCamera());
	//Renderer2D::DrawQuad({ 0,1,0 }, { 1,1,1 }, nullptr, 7, 0);
	//Renderer2D::EndScene();

	RenderCommand::SetViewport(m_ViewportSize.x, m_ViewportSize.y);
	m_Framebuffer->UnBind();
}

void SandBox2dApp::OnImGuiRender()
{
	//HZ_PROFILE_SCOPE("ImGUI RENDER");
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Color");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 displaySize = io.DisplaySize;
	//ImVec2 windowSize = ImVec2(200, 200);

	ImGui::Begin("Viewport",NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	//ImGui::SetWindowPos(ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f), ImGuiCond_Always);
	ImVec2 Size = ImGui::GetContentRegionAvail();
	if (m_ViewportSize != *(glm::vec2*)&Size)
	{
		m_ViewportSize = { Size.x,Size.y };
		m_Framebuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
	}
	ImGui::Image((void*)m_Framebuffer->GetSceneTextureID(), *(ImVec2*)&m_ViewportSize);
	
	// get the ImDrawList for the current window
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 window_size = ImGui::GetWindowSize();

	// get the position of the window in screen space
	ImVec2 window_pos = ImGui::GetWindowPos();
	// draw the x-axis
	float xGap = 1;
	for (int i = 0; i <= 100; i++)
	{
	xGap =100 * i;
	draw_list->AddLine(ImVec2(window_pos.x, window_pos.y + window_size.y / 2 + xGap),ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y / 2 +xGap), IM_COL32(255, 0, 0, 60), 2.0f);
	draw_list->AddLine(ImVec2(window_pos.x, window_pos.y + window_size.y / 2 - xGap), ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y / 2 - xGap), IM_COL32(255, 0, 0, 60), 2.0f);
	}
	// draw the y-axis
	float yGap = 1;
	for(int i=0;i<=100;i++)
	{
		yGap = 100 * i;
	draw_list->AddLine(ImVec2(window_pos.x + window_size.x / 2 + yGap, window_pos.y),ImVec2(window_pos.x + window_size.x / 2 + yGap, window_pos.y + window_size.y), IM_COL32(0, 255, 0, 60), 2.0f);
	draw_list->AddLine(ImVec2(window_pos.x + window_size.x / 2 - yGap, window_pos.y), ImVec2(window_pos.x + window_size.x / 2 - yGap, window_pos.y + window_size.y), IM_COL32(0, 255, 0, 60), 2.0f);
	}
	// draw the x-axis label
	draw_list->AddText(ImVec2(window_pos.x + window_size.x - 40, window_pos.y + window_size.y / 2 + 10),
		0xFF0000FF, "TIME");
	//the ImU32 format is 0xAABBGGRR where the 'A' is the alpha 'B' is the blue , 'R' is the red , 'G' is the green channel..it supports values fom 0 to 15
	// draw the y-axis label
	draw_list->AddText(ImVec2(window_pos.x + window_size.x / 2 + 10, window_pos.y + 20), 0xFF00FF00, "MONEY");
	//..................................................DRAWING MOVABLE LINES.....................................................................
	//use y= mx+c 
	//straight line parallel to y axis is x = k; where k= some constant(here mouse x position)

		auto MousePos = ImGui::GetIO().MousePos;	
		draw_list->AddLine(ImVec2(MousePos.x, 0), ImVec2(MousePos.x, 10000), IM_COL32(255, 255, 255, 100), 2.0f);

		MousePos.x -= window_pos.x;//mouse pos relative to the window
		MousePos.y -= window_pos.y;
		
		ImVec2 coord = { MousePos.x - m_ViewportSize.x / 2 ,m_ViewportSize.y / 2 - MousePos.y };//cartesian coordinate generation

		HAZEL_CORE_ERROR(coord.x);
		HAZEL_CORE_ERROR(coord.y);

		//zoom is not taken into account
	for (auto points : m_Points) {
		if(coord.y > points.y*100 -30 && coord.y < points.y*100+30  )
		draw_list->AddLine(ImVec2(0, MousePos.y+window_pos.y), ImVec2(10000, MousePos.y+ window_pos.y), IM_COL32(255, 255, 255, 100), 2.0f);
	}
	ImGui::End();

	ImGui::Begin("Curve Controller");
	ImGui::DragFloat2("point0", (float*)&P0, 0.1, -100, 100);
	ImGui::DragFloat2("point1", (float*)&P1, 0.1, -100, 100);
	ImGui::DragFloat("factor", &factor, 0.1, 0, 1);
	ImGui::End();

}

void SandBox2dApp::OnEvent(Event& e)
{
		m_camera.OnEvent(e);
}
