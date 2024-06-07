#pragma once
#include "Hazel.h"
#include "MonteCarloSim.h" 
#include <unordered_map>

#define WEEKLY "Weekly Time Series"
#define MONTHLY "Monthly Time Series"
#define DAILY "Time Series (Daily)"

using namespace Hazel;

enum APIInterval {
	_WEEKLY,
	_MONTHLY,
	_DAILY,
	_INTRADAY
};

enum GraphType {
	_LINE,
	_CANDLESTICK,
	_NONE
};
struct TradingVal {
	 std::string xLabel;
	 std::string open , close, high, low, volume;
};

class News;
//Trading software build client side
class SandBox2dApp :public Layer {
public:
	SandBox2dApp();
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltatime) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Event& e) override;
public:
	static std::vector<float> ClosingPrices;
	std::vector<float> FuturePrices;
	MonteCarloSim MCSim;
	std::string key;
private:
	News* news;
	OrthographicCameraController m_camera;
	glm::vec2 m_ViewportSize = { 0,0 };
	
	std::vector<glm::vec2> m_Points;
	std::vector<TradingVal> val;
	std::vector<std::string> CompanyDescription;
	std::vector<std::string> suggestions;//array of strings that stores the auto compleate suggestions that we receive from the server
	int selected_suggestion = 0;//stores the suggestion index that is selected in the listbox, used in Imgui::ListBox()

	glm::vec2 window_pos = {0,0};
	glm::vec2 Window_Size = { -1,-1 };
	glm::vec2 tmp_MousePos = { -1000,-1000 };
	glm::vec2 PointOnTheGraph;//stores the values of the point on the graph
	std::pair<std::string, std::string> MousePos_Label;
	glm::vec4 color = { 1,0.462,0,1 };

	std::string DataInterval = "TIME_SERIES_INTRADAY",interval=DAILY;

	std::string CompanyName="IBM";//default
	std::string tmp_string = "";//used in Draw_X_axis_Label function
	std::string SearchResult = "";//contain the search result of auto search

	APIInterval ApiInterval = APIInterval::_WEEKLY;//used in x-axis labeling
	GraphType graphtype = GraphType::_LINE;

	std::unordered_map<std::string, GraphType> GraphType_Map;
	int IndexOfGraph = 0;

	float factor = 0.0;
	float max_val = INT_MIN, min_val = INT_MAX;
	float max_volume = INT_MIN,min_volume = INT_MAX;
	float volume_scale=100;
	int coordinate_scale = 1;
	int UpscaledValue = 100; // this variable is used for scaling the normalized-finance data values
	int NumPoints = 1000;
	int SkipCoordinate = 0;//used to skip coordinate label
	int interval_in_min = 5;
	bool isFetchingData = false;
	bool isWindowFocused = false;
	bool isDatafetched = false;
	std::thread search_thread;
	std::mutex m;//LOCK VARIABLE

	ref<Scene> m_Scene;
	ref<Shader> shader;
	ref<VertexArray> vao;
	ref<SubTexture2D> tree,land,mud,water;
	ref<FrameBuffer> m_Framebuffer;

private:
	glm::vec2 normalize_data(const glm::vec2& arr_ele, const int& index);
	glm::vec2 ConvertToScreenCoordinate(glm::vec4& OGlCoordinate, glm::vec2& ViewportSize);
	void drawCurve();
	void drawCandleStick();
	void drawVolumeBarGraph(ImDrawList* draw_list);
	void FetchData();
	void AutoFill(const std::string& str);
	void MoveCameraToNearestPoint();
	void ChangeInterval(APIInterval apiinterval);
	void Draw_X_axis_Label(ImDrawList* draw_list);
	void PlotMonteCarloSimVals();
};