#define CURL_STATICLIB
#include "hzpch.h"
#include "Sandbox2dApp.h"
#include "DisplayNews.h"
#include "ImGuiInputText.h"
#include "curl/curl.h"
#include "json/json.h"

//#include "Hazel/Profiling.h"
std::vector<float> SandBox2dApp::ClosingPrices;
SandBox2dApp::SandBox2dApp()
	:Layer("Renderer2D layer"), m_camera(1366 / 768)
{
	key = "HZ6MXGUCREUBAXHT"; //default api key
	news = new News();

	//HZ_PROFILE_SCOPE("SandBox2dApp::SandBox2dApp()");
	GraphType_Map["CandleStick"] = GraphType::_CANDLESTICK;
	GraphType_Map["Line"] = GraphType::_LINE; //add more if u want to support more types of graphs

	m_camera.bCanBeRotated(false);
	m_Framebuffer = FrameBuffer::Create({ 1366,768 });
}

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

void SandBox2dApp::FetchData()//fetch data from the server and push that data to the m_Points array
{
	m_Points.clear();
	val.clear();
	Renderer2D::Init();
	ChangeInterval(APIInterval::_INTRADAY);

	//get data from a server using an api key
	//curl is used to connect to the server
	std::string result="";
	FILE* fp;

	CURL* curl;
	CURLcode res;
	std::string api = "https://www.alphavantage.co/query?function=" + DataInterval + "&symbol=" + CompanyName + "&outputsize=full&interval="+std::to_string(interval_in_min)+"min&apikey=HZ6MXGUCREUBAXHT";
	curl = curl_easy_init();
	if (curl) {
		fp = fopen("trading_data.json", "wb");
		curl_easy_setopt(curl, CURLOPT_URL, api.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (CURLE_OK != res) {
			std::cerr << "CURL error: " << res << '\n';
		}
		fclose(fp);
	}
	curl_global_cleanup();
	//std::cout << result << "\n\n";

	auto lamda_fn = [&](const std::string& file_name) {
	std::ifstream file(file_name);
	std::string line;
	Json::Value jsonData;//json file parser
	Json::Reader jsonReader;
	Json::CharReaderBuilder rbuilder;
	std::string errs;
	if (Json::parseFromStream(rbuilder,file, &jsonData,&errs))
	{
		uint32_t JsonSize = jsonData[interval].size();
		if (JsonSize == 0)
			return false;
		for (auto it = jsonData[interval].begin(); it != jsonData[interval].end(); it++)
		{
			std::string Xstring = it.name();
			float x =0;
			float y = std::stof(jsonData[interval][it.name()]["4. close"].asString());
			float vol=0;
			std::string volume_index = "";

			if (ApiInterval == APIInterval::_DAILY)
				volume_index = "6. volume";
			else
				volume_index = "5. volume";

			vol = std::stof(jsonData[interval][it.name()][volume_index.c_str()].asString());
			if (y > max_val)
				max_val = y;
			if (y < min_val)
				min_val = y;
			if (vol > max_volume)
				max_volume = vol;
			if (vol < min_volume)
				min_volume = vol;
			m_Points.push_back({ x,-y });//pushing the data onto the array
			TradingVal value = {Xstring,jsonData[interval][it.name()]["1. open"].asString(), jsonData[interval][it.name()]["4. close"].asString(),
			jsonData[interval][it.name()]["2. high"].asString(),jsonData[interval][it.name()]["3. low"].asString(),
			jsonData[interval][it.name()][volume_index.c_str()].asString() };
			val.push_back(value);
			ClosingPrices.push_back(y);
		}
		
		HAZEL_CORE_WARN(max_volume);
		HAZEL_CORE_INFO(min_volume);

		//if (JsonSize > NumPoints)
		//{
		//	m_Points.erase(m_Points.begin(), m_Points.end() - NumPoints);
		//	val.erase(val.begin(), val.end() - NumPoints);
		//}
	}

	};
	if (!lamda_fn("trading_data.json"))
		lamda_fn("default_trading_data.json");
	HAZEL_CORE_ERROR(result);
	if(m_Points.size()>0)
	m_camera.SetCameraPosition({ normalize_data(m_Points[0],0),0 });
}


//this func is used in multi threading environment
void SandBox2dApp::AutoFill(const std::string& str)//this function is used for the auto compleate searchbox logic, it connects to the server and saves the data to a json file
{
	std::lock_guard<std::mutex> lock(m);//mutex which helps in synchronization of the processes when 2 or more thread tries to access the shared code
	suggestions.clear();
	CompanyDescription.clear();

	CURL* curl;
	CURLcode res;
	FILE* fp;
	std::string api = "https://www.alphavantage.co/query?function=SYMBOL_SEARCH&keywords=" + str + "&apikey=4NT07D3RF1DRQBSL&datatype=json";
	char outfilename[FILENAME_MAX] = "data.json";
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, api.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (CURLE_OK != res) {
			std::cerr << "CURL error: " << res << '\n';
		}
		fclose(fp);
	}
	curl_global_cleanup();
	std::ifstream file("data.json");
	Json::Value jsondata;
	Json::Reader jsonreader;
	if (jsonreader.parse(file, jsondata))
	{
		if (jsondata["bestMatches"].begin() == jsondata["bestMatches"].end())
		{
			suggestions.push_back("No Matches Found");
			return;
		}
		for (auto it = jsondata["bestMatches"].begin(); it != jsondata["bestMatches"].end(); it++) {
			suggestions.push_back(it->begin()->asString());
			std::string tmp = "";
			for (auto i = it->begin(); i != it->end(); i++)
				tmp += i.name()+" : " + i->asString() + "\n";
			CompanyDescription.push_back(tmp);
		}

		HAZEL_CORE_INFO(jsondata["bestMatches"]);
	}
};

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
	m_camera.SetCameraPosition(glm::vec3(PointOnTheGraph.x, PointOnTheGraph.y, 0));//move the camera to the pont on the curve
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
	auto GetMonthNames = [=](const std::string& date) {
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
	drawVolumeBarGraph(draw_list);
}

void SandBox2dApp::PlotMonteCarloSimVals()
{
	if(FuturePrices.size()==0)
		return
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i < FuturePrices.size() - 1; i++)
			Renderer2D::DrawLine(glm::vec3(normalize_data({ m_Points[i].x,FuturePrices[i] }, i), 0), glm::vec3(normalize_data({ m_Points[i + 1].x,FuturePrices[i + 1] }, i + 1), 0), {0.2,1.0,0.3,1.0});

		Renderer2D::LineEndScene();
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
	//std::thread t1([&]() {FetchData(); });
	//std::thread t2([&]() {news->OnAttach(); });
	//t1.join();
	//t2.join();
	FetchData();
	news->OnAttach();
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

	news->OnUpdate(); //not working!!
	//draw lines at cursor tip
	//use y= mx+c 
	//straight line parallel to y axis is x = k; where k= some constant(here mouse x position)
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		auto MousePos = Input::GetCursorPosition();//get the mouse position
		auto Window_Size = RenderCommand::GetViewportSize();//get the view port dimensions

		glm::mat4 invVP = glm::inverse(m_camera.GetCamera().GetProjectionViewMatix());//inverse the view projection matrix
		//window_pos gives the imGui viewport position
		glm::vec4 screenPos = glm::vec4((MousePos.first - window_pos.x) / (Window_Size.x * 0.5) - 1.0, MousePos.second / (Window_Size.y * 0.5) - 1.0, 1.0f, 1.0f);
		glm::vec4 worldPos = invVP * screenPos;//converted mouse pos to opengl position

		auto getNearestPointToCursor = [&]() //this lamda returns a point on the graph that is nearest to the mouse position(in opengl coordinate)
		{
			float minDist = INT_MAX;
			glm::vec2 minpoint = { -1,-1 };
			for (int i = 0; i < m_Points.size(); i++)
			{
				float dist = sqrt(pow(i - worldPos.x, 2) + pow(normalize_data(m_Points[i], i).y - worldPos.y, 2));
				if (dist < minDist)
				{
					minDist = dist;
					minpoint = { i,m_Points[i].y };
				}
			}
			tmp_MousePos = { minpoint.x,m_Points[minpoint.x].y };
			MousePos_Label = { val[minpoint.x].xLabel,val[minpoint.x].close };//for drawing the mouse coordinates
			return normalize_data(minpoint, minpoint.x);
		};
		PointOnTheGraph = getNearestPointToCursor();
		Renderer2D::DrawLine({ PointOnTheGraph.x,-1000,0 }, { PointOnTheGraph.x,1000,0 }, { 1,1,1,0.9 }, 1);
		Renderer2D::DrawLine({ -10000 ,PointOnTheGraph.y,0 }, { 10000,PointOnTheGraph.y,0 }, { 1,1,1,0.9 }, 1);
	}
	Renderer2D::LineEndScene();


	//choose which type of graph to draw
	switch (graphtype) {
	case GraphType::_CANDLESTICK:
		drawCandleStick();
		break;
	case GraphType::_LINE:
		drawCurve();
		break;
	};
	if (FuturePrices.size() != 0)
	{
		Renderer2D::LineBeginScene(m_camera.GetCamera());
		{
			for (int i = 0; i < FuturePrices.size() - 1; i++) {
				//std::cout << normalize_data({ m_Points[i].x,-FuturePrices[i] }, i).y;
				Renderer2D::DrawLine(glm::vec3(normalize_data({ m_Points[i].x,-FuturePrices[i] }, i), 0), glm::vec3(normalize_data({ m_Points[i + 1].x,-FuturePrices[i + 1] }, i + 1), 0), { 0.2,1.0,0.3,1.0 });
			}//Renderer2D::DrawLine(glm::vec3(0,0, 0), glm::vec3(180,2, 0), { 0.2,1.0,0.3,1.0 },20.0f);

			Renderer2D::LineEndScene();
		}
	}

	//draw the x-axises
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	{
		for (int i = 0; i <= m_Points.size() ; i += coordinate_scale)
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

	ImGui::Begin("Viewport",NULL, ImGuiWindowFlags_NoCollapse);
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
		draw_list->AddText({ Screen_coord.x-500,Screen_coord.y }, IM_COL32(239, 0, 255, 230), &label[0]);
	}

	//rendering the coordinates of the mouse cursor (m_Points[tmp_MousePos.x])
		if (tmp_MousePos.x > -1000)
		{
			auto Pos_Mouse = io.MousePos;
			std::string label_x = MousePos_Label.first;
			draw_list->AddText({ Pos_Mouse.x + 15,Pos_Mouse.y + 50 }, IM_COL32(255, 255, 255, 255), &label_x[0]);//slight offset is given
			std::string label_y = " , "+ MousePos_Label.second;
			draw_list->AddText({ Pos_Mouse.x + label_x.size() * 8 ,Pos_Mouse.y +50 }, IM_COL32(255, 255, 255, 255), &label_y[0]);
		}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Properties");
	if (ImGui::Button("Run MonteCarlo Simulation"))
	{
		FuturePrices = MCSim.RunSimulation(1, 100, 300, 200);
	}
	ImGui::DragFloat("Curvature Factor", &factor, 0.1, 0, 1);
	ImGui::DragInt("Coordinate Spacing", &coordinate_scale, 1, 1, 10);
	ImGui::DragFloat("Volume Scale", &volume_scale, 10, 0, 500000);
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
	ImGuiInputTextCallback callback ;
	if (ImGui::InputText("Company",&CompanyName))
	{
		//std::this_thread::sleep_for(std::chrono::seconds(2));
		search_thread = std::thread([=]() {AutoFill(CompanyName); });
		search_thread.detach();
	}


	if (suggestions.size() > 0)//draw suggestion list if suggestions vector has some values
	{
		const char* list[100];
		if (suggestions.size() < 100)//just a protection
			for (int i = 0; i < suggestions.size(); i++)
				list[i] = suggestions[i].c_str();
		else
			for (int i = 0; i < 100; i++)
				list[i] = suggestions[i].c_str();

		if (ImGui::ListBox(" ", &selected_suggestion, list, suggestions.size()))
		{
			CompanyName = suggestions[selected_suggestion];
		}
		if (ImGui::IsItemHovered())
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
			ImGui::SetNextWindowPos({ io.MousePos.x + 20,io.MousePos.y });
			ImGui::SetNextWindowSize({ 300,200 });
			ImGui::Begin(" ");
			if (selected_suggestion < CompanyDescription.size())
				ImGui::Text(CompanyDescription[selected_suggestion].c_str());
			ImGui::End();
		}
	}

	if (ImGui::Button("APPLY", { 150,20 }))
	{
		//std::thread t(std::mem_fn(&SandBox2dApp::FetchData), this);
		//t.join();
		FetchData();
	}
	{
		const char*const list[2] = { "Line","CandleStick" };//names must be same as the GraphType_Map(unordered_map) key values
		if (ImGui::Combo("Graph Types", &IndexOfGraph, list, 2)) 
			Renderer2D::Init();//to clear the previous drawn buffer
		graphtype = GraphType_Map[list[IndexOfGraph]];
	}
	ImGui::End();

	ImGui::Begin("Graph color");
	ImGui::ColorPicker4("PickColor", (float*)&color);
	ImGui::End();

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);

	//ImGui::SetNextWindowPos({ 1000,20});
	ImGui::Begin("Values");
	ImGui::TextColored({1,1,1,1}, (CompanyName + "  (" + interval + ")").c_str());

	float netchange = -(m_Points[m_Points.size() - 1].y - m_Points[m_Points.size() - 2].y);//the y value is inverted for drawing in opengl
	float NetChangePercent = ((m_Points[m_Points.size() - 1].y / m_Points[m_Points.size() - 2].y) - 1) * 100;
	if (netchange > 0)//net cng formula current price - old price
		ImGui::TextColored({0,1,0.2,1}, ("  +" + std::to_string(netchange).substr(0, 6) + "  (+" + std::to_string(NetChangePercent).substr(0, 6) + " %%)").c_str());
	else
		ImGui::TextColored({ 1,0,0.2,1 }, ("  " + std::to_string(netchange).substr(0, 6) + "  (" + std::to_string(NetChangePercent).substr(0,6) + " %%)").c_str());

	if (tmp_MousePos.x > -1000 && tmp_MousePos.x<val.size())//when the interval is changed the tmp_MousePos is not updated so we put a check here to resolve any unwanted errors
	{
		int i = tmp_MousePos.x;
		glm::vec4 color = { 1,1,1,1 };
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

		if (tmp_MousePos.x > 0) {
			float netchange = -(m_Points[tmp_MousePos.x].y - m_Points[tmp_MousePos.x - 1].y);//the y value is inverted for drawing in opengl
			float NetChangePercent = ((m_Points[tmp_MousePos.x].y / m_Points[tmp_MousePos.x - 1].y) - 1) * 100;
			if (netchange > 0)//net cng formula current price - old price
				ImGui::TextColored({ 0,1,0.2,1 }, ("Change:  +" + std::to_string(netchange).substr(0, 6) + "  (+" + std::to_string(NetChangePercent).substr(0, 6) + " %%)").c_str());
			else
				ImGui::TextColored({ 1,0,0.2,1 }, ("Change:  " + std::to_string(netchange).substr(0, 6) + "  (" + std::to_string(NetChangePercent).substr(0, 6) + " %%)").c_str());
		}

	}
	ImGui::End();

	news->OnImGuiRender();
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
		dispatch.Dispatch<KeyPressedEvent>([&](KeyPressedEvent e) {
			if (e.GetKeyCode() == HZ_KEY_R)
				m_camera.SetCameraPosition(glm::vec3(m_Points.size(),normalize_data(m_Points[m_Points.size()-1],m_Points.size() - 1).y,0));
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
		//if (factor == 0)//if factor is ==0 no need to draw curved lines 
			for (int i = 0; i < m_Points.size() - 1; i++) {
				//std::cout << normalize_data(m_Points[i], i).y;
				Renderer2D::DrawLine(glm::vec3(normalize_data(m_Points[i], i), 0), glm::vec3(normalize_data(m_Points[i + 1], i + 1), 0), color);
			}
		//else {
		//	for (int i = 0; i < m_Points.size() - 1; i++) 
		//	{
		//		auto x = velocity(tmp_point, normalize_data(m_Points[i + 1], i + 1), factor);
		//		if (tmp_point.x > 0)
		//		Renderer2D::DrawCurve(tmp_point, normalize_data(m_Points[i], i), tmp_vel, x, color);//drawing a hermitian curve
		//		tmp_point = normalize_data(m_Points[i], i);
		//		tmp_vel = x;
		//	}
		//	Renderer2D::DrawCurve(normalize_data(m_Points[m_Points.size() - 2], m_Points.size() - 2), normalize_data(m_Points[m_Points.size()-1], m_Points.size() - 1), tmp_vel, tmp_vel,color);
		//}
	Renderer2D::LineEndScene();
	}
}
void SandBox2dApp::drawCandleStick()
{
	glm::vec4 Lcolor = { 0,1,0,1 };
	float Low, High, Open, Close;
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	for (int i = 0; i < val.size(); i++)
	{
		Low = -stof(val[i].low);
		High = -stof(val[i].high);
		Close = -stof(val[i].close);
		Open = -stof(val[i].open);
		if (-Close < -Open)
			Lcolor = { 1,0,0,1 };
		else
			Lcolor = { 0,1,0,1 };
		Renderer2D::DrawLine({ i,normalize_data({i,Open},i).y,0 }, { i,normalize_data({i,Close},i).y,0 }, Lcolor,10);
	}
	Renderer2D::LineEndScene();
	Renderer2D::LineBeginScene(m_camera.GetCamera());
	for (int i = 0; i < val.size(); i++)
	{
		Low = -stof(val[i].low);
		High = -stof(val[i].high);
		Close = -stof(val[i].close);
		Open = -stof(val[i].open);
		Renderer2D::DrawLine({ i,normalize_data({i,Low},i).y,0 }, { i,normalize_data({i,High},i).y,0 }, { 1,1,1,1 },2);
	}
	Renderer2D::LineEndScene();
}

void SandBox2dApp::drawVolumeBarGraph(ImDrawList* draw_list) {

	float volume;
	auto LColour = IM_COL32(200, 255, 0, 150);
	float thickness=8;
	float zoom = m_camera.GetZoomLevel();
	if (zoom <= 17 && zoom >= 10)
		thickness = 15;
	if (zoom < 10 && zoom>=4)
		thickness = 35;
	if (zoom < 4 && zoom>=2)
		thickness = 45;
	if (zoom < 2)
		thickness = 65;
	for (int i = 1; i < val.size(); i++)
	{
		volume = (-stof(val[i].volume) - min_volume) / (max_volume - min_volume);
		volume *= volume_scale;
		glm::vec4 world_coord = { i - m_camera.GetPosition().x,0,0,0 };
		auto ScreenCoord = ConvertToScreenCoordinate(world_coord, Window_Size);
		if (-m_Points[i - 1].y <= -m_Points[i].y)
			LColour = IM_COL32(120, 255, 0, 120);
		else
			LColour = IM_COL32(255, 0, 80, 120);
		draw_list->AddLine({ ScreenCoord.x,ScreenCoord.y+300 }, { ScreenCoord.x,ScreenCoord.y+volume+300}, LColour, thickness);
	}

}
