#include "Sandbox2dApp.h"
//#include "Hazel/Profiling.h"

SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1366 / 768)
{
	//HZ_PROFILE_SCOPE("SandBox2dApp::SandBox2dApp()");

	m_Framebuffer = FrameBuffer::Create({ 1366,768 });
	Renderer2D::Init();
}

void SandBox2dApp::OnAttach()
{
	m_Points = { { 5,5 },{8,8},{10,0} ,{12,3},{15,2},{16,3},{18,2.1},{18.2,3.5},{19,3.9},{19.8,-5},{20.5,2},{22,3},{25,-10},{26,-8},{29,3},{33,7} ,{34,2} };//array of points for testing
	for (int i = 0; i < m_Points.size(); i++)
		m_Points[i].y *= -1;//invert the y axis as open gl inverts the y axis
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

	auto CheckPointInRadius = [&](const glm::vec2& p1, const glm::vec2& p2,float radius) 
	{
		float d_x = p2.x - p1.x;
		float d_y = p2.y - p1.y;
		float distance = d_x * d_x + d_y * d_y;
		if (distance <= radius * radius)
			return true;
		else
			return false;
	};

	//draw lines at cursor tip
	//use y= mx+c 
	//straight line parallel to y axis is x = k; where k= some constant(here mouse x position)
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		auto MousePos = Input::GetCursorPosition();//get the mouse position
		auto Window_Size = RenderCommand::GetViewportSize();//get the view port dimensions
		glm::mat4 invVP = glm::inverse(m_camera.GetCamera().GetProjectionViewMatix());
		//window_pos gives the imGui viewport position
		glm::vec4 screenPos = glm::vec4((MousePos.first - window_pos.x) / (Window_Size.x * 0.5) - 1.0, MousePos.second / (Window_Size.y * 0.5) - 1.0, 1.0f, 1.0f);
		glm::vec4 worldPos = invVP * screenPos;

		Renderer2D::DrawLine({ worldPos.x,-1000,0 }, { worldPos.x,1000,0 }, { 1,1,1,0.6 }, 1.5);//kind a works

		for (int i=0;i<m_Points.size();i++)
		{
			auto actualcoord = m_camera.GetCamera().GetProjectionViewMatix() * glm::vec4(m_Points[i], 1.0, 1.0);
			if (CheckPointInRadius(m_Points[i], worldPos, 1)) {
				Renderer2D::DrawLine({ -1000 ,m_Points[i].y,0 }, { 1000,m_Points[i].y,0 }, { 1,1,1,0.6 }, 1.5);//kind a works
				tmp_MousePos = m_Points[i];
				break;
			}
			else
				tmp_MousePos = { -1000,-1000 };
			//else if(i-1>0)
				//Renderer2D::DrawLine({ -1000 ,m_Points[i-1].y,0 }, { 1000,m_Points[i-1].y,0 }, { 0.3,0.5,0.9,1 }, 1.5);
		}
	}
	Renderer2D::LineEndScene();

	glm::vec2 tmp_point = P1, tmp_vel = velocity(P0, m_Points[0], factor);
	//draw the curves from the coordinates of the array m_Points
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < m_Points.size() - 1; i++)
		{
			auto x = velocity(tmp_point, m_Points[i + 1], factor);
			Renderer2D::DrawCurve(tmp_point, m_Points[i], tmp_vel, x, { 0,0.5,0.7,1 });//drawing a hermitian curve
			tmp_point = m_Points[i];
			tmp_vel = x;
		}
	}
	Renderer2D::LineEndScene();


	//draw the y-axises
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < 100 * coordinate_scale; i += coordinate_scale)
			Renderer2D::DrawLine({ i,-1000.0f,0 }, { i,1000.0f,0 }, { 0,1,0,0.4 }, 1);
		for (int i = -100 * coordinate_scale; i < 0; i += coordinate_scale)
			Renderer2D::DrawLine({ i,-1000.0f,0 }, { i,1000.0f,0 }, { 0,1,0,0.4 }, 1);
	}
	Renderer2D::LineEndScene();


	//draw the x-axises
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < 100 * coordinate_scale; i += coordinate_scale)
			Renderer2D::DrawLine({ -1000,i,0 }, { 1000,i,0 }, { 1,0,0,0.4 }, 1);
		for (int i = -1000 * coordinate_scale; i < 0; i += coordinate_scale)
			Renderer2D::DrawLine({ -1000,i,0 }, { 1000,i,0 }, { 1,0,0,0.4 }, 1);
	}
	Renderer2D::LineEndScene();


	RenderCommand::SetViewport(m_ViewportSize.x, m_ViewportSize.y);
	m_Framebuffer->UnBind();
}


void SandBox2dApp::OnImGuiRender()
{
	auto ConvertToScreenCoordinate = [&](glm::vec4& OGlCoordinate,glm::vec2& ViewportSize) //this function converts opengl coordinate to screen coordinate
	{
		OGlCoordinate = OGlCoordinate * m_camera.GetCamera().GetProjectionViewMatix();
		float x = (OGlCoordinate.x + 1) * ViewportSize.x * 0.5 + window_pos.x;//transforming into window coordinate
		float y = (OGlCoordinate.y + 1) * ViewportSize.y * 0.5 + window_pos.y;
		return glm::vec2(x, y);
	};


	//HZ_PROFILE_SCOPE("ImGUI RENDER");
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Color");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 displaySize = io.DisplaySize;
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));//to remove the borders

	ImGui::Begin("Viewport",NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

		window_pos = *(glm::vec2*)&ImGui::GetWindowPos();

	ImVec2 Size = ImGui::GetContentRegionAvail();
	if (m_ViewportSize != *(glm::vec2*)&Size)
	{
		m_Framebuffer->Resize((unsigned int)Size.x, (unsigned int)Size.y);
		m_ViewportSize = { Size.x,Size.y };
		m_camera.onResize(Size.x, Size.y);//resize the camera
		RenderCommand::SetViewport(Size.x, Size.y);
	}
	ImGui::Image((void*)m_Framebuffer->GetSceneTextureID(), {m_ViewportSize.x,m_ViewportSize.y});

	
	// get the ImDrawList for the current window
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	//ImVec2 window_size = ImGui::GetWindowSize();
	auto Window_Size = RenderCommand::GetViewportSize();

	//x-axis coordinate label
	for (int i = 0; i < 100*coordinate_scale; i+=coordinate_scale)
	{
		glm::vec4 worldPos = { i - m_camera.GetPosition().x ,0 - m_camera.GetPosition().y,0,0 };//it is the openGl coordinate
		auto Screen_coord = ConvertToScreenCoordinate(worldPos, Window_Size);//convert opengl coordinate to screen coordinate(i.e 1366,720)
		auto label = std::to_string(i);
		draw_list->AddText({ Screen_coord.x,Screen_coord.y }, IM_COL32(0,255,0,230), &label[0]);
	}

	//y-axis coordinate label
	for (int i = -100* coordinate_scale; i < 100 * coordinate_scale; i += coordinate_scale)
	{
		glm::vec4 worldPos = { 0 ,i - m_camera.GetPosition().y,0,0 };//it is the openGl coordinate
		auto Screen_coord = ConvertToScreenCoordinate(worldPos, Window_Size);//convert opengl coordinate to screen coordinate(i.e 1366,720)
		auto label = std::to_string(-i);//invert the y-axis
		draw_list->AddText({ Screen_coord.x,Screen_coord.y }, IM_COL32(255, 0, 0, 230), &label[0]);
	}

	//rendering the coordinates of the mouse cursor
		if (tmp_MousePos.x > -1000)
		{
			auto Pos_Mouse = io.MousePos;
			std::string label_x = std::to_string(tmp_MousePos.x);
			draw_list->AddText({ Pos_Mouse.x + 15,Pos_Mouse.y + 15 }, IM_COL32(255, 255, 255, 255), &label_x[0]);//slight offset is given
			std::string label_y = " , " + std::to_string(-tmp_MousePos.y);

			draw_list->AddText({ Pos_Mouse.x + 90,Pos_Mouse.y + 15 }, IM_COL32(255, 255, 255, 255), &label_y[0]);
		}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Curve Controller");
	ImGui::DragFloat2("point0", (float*)&P0, 0.1, -100, 100);
	ImGui::DragFloat2("point1", (float*)&P1, 0.1, -100, 100);
	ImGui::DragFloat("factor", &factor, 0.1, 0, 1);
	ImGui::DragInt("Coordinate Spacing", &coordinate_scale, 1, 1, 10);
	ImGui::End();

}

void SandBox2dApp::OnEvent(Event& e)
{
		m_camera.OnEvent(e);
}
