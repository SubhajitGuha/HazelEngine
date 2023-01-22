#define CURL_STATICLIB
#include "hzpch.h"
#include "Sandbox2dApp.h"
#include "curl/curl.h"
#include "json/json.h"

//#include "Hazel/Profiling.h"

SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1366 / 768)
{
	//HZ_PROFILE_SCOPE("SandBox2dApp::SandBox2dApp()");
	m_camera.bCanBeRotated(false);

	m_Framebuffer = FrameBuffer::Create({ 1366,768 });
	//Renderer2D::Init();
}


static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
	std::string& text = *static_cast<std::string*>(param);
	size_t totalsize = size * nmemb;
	text.append(static_cast<char*>(buffer), totalsize);
	return totalsize;
}

void SandBox2dApp::FetchData()//fetch data from the server and push that data to the m_Points array
{
	m_Points.clear();
	val.clear();
	Renderer2D::Init();

	//get data from a server using an api key
	//curl is used to connect to the server
	std::string result="";
	CURL* curl;
	CURLcode res;
	std::string api = "https://www.alphavantage.co/query?function=" + DataInterval + "&symbol=" + CompanyName + "&outputsize=full&interval="+std::to_string(interval_in_min)+"min&apikey=4NT07D3RF1DRQBSL";
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, api.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (CURLE_OK != res) {
			std::cerr << "CURL error: " << res << '\n';
		}
	}
	curl_global_cleanup();
	std::cout << result << "\n\n";
	Json::Value jsonData;//json file parser
	Json::Reader jsonReader;

	if (jsonReader.parse(result, jsonData)) 
	{
		uint32_t JsonSize = jsonData[interval].size();

		for (auto it = jsonData[interval].begin(); it != jsonData[interval].end(); it++)
		{

			std::string Xstring = it.name();
			float x =0;
			float y = std::stof(jsonData[interval][it.name()]["4. close"].asString());
			if (y > max_val)
				max_val = y;
			if (y < min_val)
				min_val = y;
			m_Points.push_back({ x,-y });//pushing the data onto the array
			TradingVal value = {Xstring,jsonData[interval][it.name()]["1. open"].asString(), jsonData[interval][it.name()]["4. close"].asString(),
			jsonData[interval][it.name()]["2. high"].asString(),jsonData[interval][it.name()]["3. low"].asString(),
			jsonData[interval][it.name()]["5. volume"].asString() };
			val.push_back(value);
		}
		if (JsonSize > NumPoints)
		{
			m_Points.erase(m_Points.begin(), m_Points.end() - NumPoints);
			val.erase(val.begin(), val.end() - NumPoints);
		}
	}
	m_camera.SetCameraPosition({ normalize_data(m_Points[m_Points.size() - 1],m_Points.size() - 1),0 });
}

auto CheckPointInRadius = [&](const glm::vec2& p1, const glm::vec2& p2, float radius)
{
	float d_x = p2.x - p1.x;
	float d_y = p2.y - p1.y;
	float distance = d_x * d_x + d_y * d_y;
	if (distance <= radius * radius)
		return true;
	else
		return false;
};

void SandBox2dApp::MoveCameraToNearestPoint()
{
	auto CalculateDistance = [&](const glm::vec2& p1 , const glm::vec2& p2) {
		float tmp;
		tmp = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
		return tmp;
	};

	float dist;
	glm::vec3 CamPos = m_camera.GetPosition();
	glm::vec2 MoveToPos=normalize_data(m_Points[m_Points.size()-1],m_Points.size()-1);
	for (int i = m_Points.size() - 1; i >= 0; i--)
	{
		glm::vec2 point = normalize_data(m_Points[i], i);
		if (CheckPointInRadius({ CamPos.x,CamPos.y }, point, 50))
		{
			MoveToPos = point;
			break;
		}

	}
	m_camera.SetCameraPosition(glm::vec3(m_camera.GetPosition().x,MoveToPos.y, 0));
}

void SandBox2dApp::ChangeInterval(APIInterval apiinterval)
{
	ApiInterval = apiinterval;
	switch (apiinterval)
	{
	case APIInterval::_WEEKLY:
		DataInterval = "TIME_SERIES_WEEKLY";
		interval = WEEKLY;
		return;
	case APIInterval::_MONTHLY:
		DataInterval = "TIME_SERIES_MONTHLY";
		interval = MONTHLY;
		return;
	case APIInterval::_DAILY:
		DataInterval = "TIME_SERIES_DAILY_ADJUSTED";
		interval = DAILY;
		return;
	case APIInterval::_INTRADAY:
		DataInterval = "TIME_SERIES_INTRADAY";
		interval = "Time Series (" + std::to_string(interval_in_min) + "min)";
		return;
	}
}


void SandBox2dApp::Draw_X_axis_Label(ImDrawList* draw_list)
{
	auto GetMonthNames = [=](std::string date) {
		int monthNum = stoi(date.substr(5, 7));
		switch (monthNum) {
		case 1:
			return "Jan" + date.substr(7);
		case 2:
			return "Feb" + date.substr(7);
		case 3:
			return "Mar" + date.substr(7);
		case 4:
			return "Apr" + date.substr(7);
		case 5:
			return "May" + date.substr(7);
		case 6:
			return "Jun" + date.substr(7);
		case 7:
			return "Jul" + date.substr(7);
		case 8:
			return "Aug" + date.substr(7);
		case 9:
			return "Sep" + date.substr(7);
		case 10:
			return "Oct" + date.substr(7);
		case 11:
			return "Nov" + date.substr(7);
		case 12:
			return "Dec" + date.substr(7);
		}
	};

	for (int i = 0; i < m_Points.size(); i+=SkipCoordinate+ coordinate_scale) {
		glm::vec4 worldPos = { i - m_camera.GetPosition().x ,0,0,0 };//it is the openGl coordinate
		auto Screen_coord = ConvertToScreenCoordinate(worldPos, Window_Size);//convert opengl coordinate to screen coordinate(i.e 1366,720)

		auto label = val[i].xLabel;
		switch (ApiInterval)
		{
		case APIInterval::_WEEKLY :
		case APIInterval::_MONTHLY :
		case APIInterval::_DAILY:
			if (label.substr(0, 4) != tmp_string.substr(0, 4))
			{
				auto sublabel = label.substr(0, 4);
				draw_list->AddText({ Screen_coord.x,Screen_coord.y + 300 }, IM_COL32(0, 225, 255, 230), &sublabel[0]);
				tmp_string = label;
			}
			else
			{
				auto sublabel = GetMonthNames(label);
				draw_list->AddText({ Screen_coord.x,Screen_coord.y + 300 }, IM_COL32(0, 255, 0, 230), &sublabel[0]);
			}
			break;
		case APIInterval::_INTRADAY:
			if (label.substr(0, 10) != tmp_string.substr(0, 10))
			{
				auto sublabel = label.substr(0, 4);
				draw_list->AddText({ Screen_coord.x,Screen_coord.y + 320 }, IM_COL32(0, 225, 255, 230), &label[0]);
				tmp_string = label;
			}
			else
			{
				auto sublabel = label.substr(10);
				draw_list->AddText({ Screen_coord.x,Screen_coord.y + 300 }, IM_COL32(0, 255, 0, 230), &sublabel[0]);
			}
			break;
		}
	}
	
}

glm::vec2 SandBox2dApp::ConvertToScreenCoordinate(glm::vec4& OGlCoordinate, glm::vec2& ViewportSize) //this function converts opengl coordinate to screen coordinate
{
	OGlCoordinate = OGlCoordinate * m_camera.GetCamera().GetProjectionViewMatix();
	float x = (OGlCoordinate.x + 1) * ViewportSize.x * 0.5 + window_pos.x;//transforming into window coordinate
	float y = (OGlCoordinate.y + 1) * ViewportSize.y * 0.5 + window_pos.y;
	return glm::vec2(x, y);
};

void SandBox2dApp::OnAttach()
{
	FetchData();
}

void SandBox2dApp::OnDetach()
{
	delete[] & m_Points[0];
	delete[] & val[0];
}

void SandBox2dApp::OnUpdate(float deltatime)
{
	m_Framebuffer->Bind();
	RenderCommand::ClearColor({ 0,0,0,1 });
	RenderCommand::Clear();

	if (isWindowFocused)
		m_camera.OnUpdate(deltatime);

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

		Renderer2D::DrawLine({ worldPos.x,-1000,0 }, { worldPos.x,1000,0 }, { 1,1,1,0.9 }, 1.5);//kind a works

		for (int i=0;i<m_Points.size();i++)
		{
			auto actualcoord = m_camera.GetCamera().GetProjectionViewMatix() * glm::vec4(m_Points[i], 1.0, 1.0);
			if (CheckPointInRadius(normalize_data(m_Points[i],i), worldPos, 1)) {
				Renderer2D::DrawLine({ -100000 ,normalize_data(m_Points[i],i).y,0 }, { 100000,normalize_data(m_Points[i],i).y,0 }, { 1,1,1,0.9 }, 1.5);//kind a works
				tmp_MousePos = { i,m_Points[i].y };
				MousePos_Label = { val[i].xLabel,val[i].close };//for drawing the mouse coordinates
				break;
			}
			else
				tmp_MousePos = { -1000,-1000 };
			//else if(i-1>0)
				//Renderer2D::DrawLine({ -1000 ,m_Points[i-1].y,0 }, { 1000,m_Points[i-1].y,0 }, { 0.3,0.5,0.9,1 }, 1.5);
		}
	}
	Renderer2D::LineEndScene();

	drawCurve();//draws the curve using the coordinates

	//draw the x-axises
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < m_Points.size()+100 ; i += coordinate_scale)
			Renderer2D::DrawLine({ i,-1000.0f+m_camera.GetPosition().y,0 }, { i,1000.0f+ m_camera.GetPosition().y,0 }, { 1,1,1,0.2 }, 1);
	}
	Renderer2D::LineEndScene();


	//draw the y-axises
	//draw from 0 to UpscaledValue * coordinate_scale
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < 10 * coordinate_scale; i += coordinate_scale)
			Renderer2D::DrawLine({ -10000+ m_camera.GetPosition().x,i,0 }, { 10000+ m_camera.GetPosition().x,i,0 }, { 1,1,1,0.2 }, 1);
		for (int i = -UpscaledValue*coordinate_scale ; i < 0; i += coordinate_scale)
			Renderer2D::DrawLine({ -10000+ m_camera.GetPosition().x,i,0 }, { 10000+ m_camera.GetPosition().x,i,0 }, { 1,1,1,0.2 }, 1);
	}
	Renderer2D::LineEndScene();

	RenderCommand::SetViewport(m_ViewportSize.x, m_ViewportSize.y);
	m_Framebuffer->UnBind();
}


void SandBox2dApp::OnImGuiRender()
{
	//HZ_PROFILE_SCOPE("ImGUI RENDER");
	ImGui::DockSpaceOverViewport();
	
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 displaySize = io.DisplaySize;
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));//to remove the borders

	ImGui::Begin("Viewport",NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		isWindowFocused = ImGui::IsWindowFocused();
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

	Window_Size = RenderCommand::GetViewportSize();

	//the labeling must be modified so that it can show all intervals like(1min,1hr,1mo,1yr)
	//x-axis coordinate label
	Draw_X_axis_Label(draw_list);

	//y-axis coordinate label
	//FINAL ADJUSTMENT...
	for (int i = 0; i < UpscaledValue * coordinate_scale; i += coordinate_scale+SkipCoordinate)
	{
		auto deNormalize = [=](const int& n) //de Normalize the finance data
		{
			float y_val = (((double)i / (UpscaledValue*coordinate_scale)) * (max_val - min_val)) + min_val;
			return y_val;
		};
		glm::vec4 worldPos = { 0 ,-i - m_camera.GetPosition().y,0,0 };//it is the openGl coordinate
		auto Screen_coord = ConvertToScreenCoordinate(worldPos, Window_Size);//convert opengl coordinate to screen coordinate(i.e 1366,720)
		auto label = std::to_string(deNormalize(i));//invert the y-axis
		draw_list->AddText({ Screen_coord.x-500,Screen_coord.y }, IM_COL32(255, 0, 0, 230), &label[0]);
	}

	//rendering the coordinates of the mouse cursor (m_Points[tmp_MousePos.x])
		if (tmp_MousePos.x > -1000)
		{
			auto Pos_Mouse = io.MousePos;
			std::string label_x = MousePos_Label.first;
			draw_list->AddText({ Pos_Mouse.x + 15,Pos_Mouse.y + 50 }, IM_COL32(255, 255, 255, 255), &label_x[0]);//slight offset is given
			std::string label_y = " , "+ MousePos_Label.second;
			draw_list->AddText({ Pos_Mouse.x + label_x.size() * 10 ,Pos_Mouse.y +50 }, IM_COL32(255, 255, 255, 255), &label_y[0]);
		}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Properties");
	ImGui::DragFloat("Curvature Factor", &factor, 0.1, 0, 1);
	ImGui::DragInt("Coordinate Spacing", &coordinate_scale, 1, 1, 10);
	if (ImGui::Button("WEEKLY", { 150,20 }))
	{
		ChangeInterval(APIInterval::_WEEKLY);
		FetchData();
		HAZEL_CORE_ERROR(m_Points.size());
	}
	if (ImGui::Button("MONTHLY", { 150,20 }))
	{
		ChangeInterval(APIInterval::_MONTHLY);
		FetchData();
		HAZEL_CORE_ERROR(m_Points.size());
	}
	if (ImGui::Button("DAILY", { 150,20 }))
	{
		ChangeInterval(APIInterval::_DAILY);
		FetchData();
		HAZEL_CORE_ERROR(m_Points.size());

	}
	if (ImGui::Button("5min Interval", { 150,20 }))
	{
		interval_in_min = 5;
		ChangeInterval(APIInterval::_INTRADAY);
		FetchData();
	}
	if (ImGui::Button("15min Interval", { 150,20 }))
	{
		interval_in_min = 15;
		ChangeInterval(APIInterval::_INTRADAY);
		FetchData();
	}
	if (ImGui::Button("1hr Interval", { 150,20 }))
	{
		interval_in_min = 60;
		ChangeInterval(APIInterval::_INTRADAY);
		FetchData();
	}
	ImGui::InputText("Company", &CompanyName[0], 200);
	if (ImGui::Button("APPLY", { 150,20 }))
	{
		//std::thread t(std::mem_fn(&SandBox2dApp::FetchData), this);
		//t.join();
		FetchData();
	}

	ImGui::End();

	ImGui::Begin("Graph color");
	ImGui::ColorPicker4("PickColor", (float*)&color);
	ImGui::End();

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);

	//ImGui::SetNextWindowPos({ 1000,20});
	ImGui::Begin("Values");
	ImGui::TextColored(*(ImVec4*)&color, CompanyName.c_str());
	if (tmp_MousePos.x > -1000)
	{
		int i = tmp_MousePos.x;
		std::string date = "Date : " + val[i].xLabel;
		ImGui::TextColored(*(ImVec4*)&color,date.c_str());
		std::string open = "Open : " + val[i].open;
		ImGui::TextColored(*(ImVec4*)&color, open.c_str());
		std::string high = "High : " + val[i].high;
		ImGui::TextColored(*(ImVec4*)&color, high.c_str());
		std::string low = "Low : " + val[i].low;
		ImGui::TextColored(*(ImVec4*)&color, low.c_str());
		std::string close = "Close : " + val[i].close;
		ImGui::TextColored(*(ImVec4*)&color, close.c_str());
		std::string volume = "volume : " + val[i].volume;
		ImGui::TextColored(*(ImVec4*)&color, volume.c_str());
	}
	ImGui::End();
}

void SandBox2dApp::OnEvent(Event& e)
{
	if (isWindowFocused)
	{
		m_camera.OnEvent(e);
		EventDispatcher dispatch(e);
		dispatch.Dispatch<MouseButtonPressed>([&](MouseButtonPressed e) {
			if(e.GetMouseButton() == HZ_MOUSE_BUTTON_2)
			MoveCameraToNearestPoint();
			return true;
			});
		dispatch.Dispatch<MouseScrollEvent>([&](MouseScrollEvent e) {
			auto zoom = m_camera.GetZoomLevel();
			if (coordinate_scale > 1)
			{
				SkipCoordinate = 0;
				return true;
			}
			if (zoom > 6 && zoom <= 20)
				SkipCoordinate = 2;
			else if (zoom > 20)
				SkipCoordinate = 4;
			else
				SkipCoordinate = 0;
			return true; });
	}
}

glm::vec2 SandBox2dApp::normalize_data(const glm::vec2& arr_ele, const int& index)//this function gives the normalized finance data (using min-max normalization)
{
	glm::vec2 tmp;
	tmp.x = index;
	//the value of arr_ele is nevative as open gl inverts y-axis
	tmp.y = (-arr_ele.y - min_val) / (max_val - min_val);//get normalized y axis value
	tmp.y *= -UpscaledValue*coordinate_scale;//again multiply the value of y with -1 to get the opengl y axis value
	//coordinate_scale is multiplied to scale the y-axis along with the coordinate scale
	return tmp;
};

void SandBox2dApp::drawCurve()
{
	auto velocity = [&](const glm::vec2& point1, const glm::vec2& point2, float& s) {
		glm::vec2 vel;
		vel = point2 - point1;
		vel.x *= s;
		vel.y *= s;
		return vel;
	};

	glm::vec2 tmp_point = { -1.0f,0.0f }, tmp_vel = velocity({-2.0f,0.0f}, normalize_data(m_Points[0], 0.0f), factor);
	//draw the curves from the coordinates of the array m_Points
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < m_Points.size() - 1; i++)
		{
			if (factor == 0)
				Renderer2D::DrawLine(glm::vec3(normalize_data(m_Points[i], i), 0), glm::vec3(normalize_data(m_Points[i + 1], i + 1), 0), color);
			else {
				auto x = velocity(tmp_point, normalize_data(m_Points[i + 1],i+1), factor);
				if(tmp_point.x>0)
				Renderer2D::DrawCurve(tmp_point, normalize_data(m_Points[i],i), tmp_vel, x, color);//drawing a hermitian curve
				tmp_point = normalize_data(m_Points[i],i);
				tmp_vel = x;
			}
		}
		if(factor>0)
			Renderer2D::DrawCurve(normalize_data(m_Points[m_Points.size() - 2], m_Points.size() - 2), normalize_data(m_Points[m_Points.size()-1], m_Points.size() - 1), tmp_vel, tmp_vel,color);
	Renderer2D::LineEndScene();
	}

}