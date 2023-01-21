#pragma once
#include "Hazel.h"
#include <unordered_map>

#define WEEKLY "Weekly Time Series"
#define MONTHLY "Monthly Time Series"
#define DAILY "Time Series (Daily)"
using namespace Hazel;

enum APIInterval {
	_WEEKLY,
	_MONTHLY,
	_DAILY
};

//Trading software build client side
class SandBox2dApp :public Layer {
public:
	SandBox2dApp();
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltatime) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Event& e) override;

private:
	OrthographicCameraController m_camera;
	glm::vec2 m_ViewportSize = { 0,0 };
	
	std::vector<glm::vec2> m_Points;
	glm::vec2 window_pos = {0,0};
	glm::vec2 tmp_MousePos = { -1000,-1000 };
	glm::vec4 color = { 1,0.462,0,1 };

	std::string DataInterval = "TIME_SERIES_WEEKLY",interval=WEEKLY;

	std::string CompanyName="IBM";//default
	float factor = 0.2;
	float max_val = INT_MIN, min_val = INT_MAX;
	int coordinate_scale = 1;
	int UpscaledValue = 100; // this variable is used for scaling the normalized-finance data values
	int NumPoints = 1000;
	bool isFetchingData = false;
	bool isWindowFocused = false;

	ref<Scene> m_Scene;
	ref<Shader> shader;
	ref<VertexArray> vao;
	ref <Texture2D> texture, tex2;
	ref<SubTexture2D> tree,land,mud,water;
	ref<FrameBuffer> m_Framebuffer;

private:
	glm::vec2 normalize_data(const glm::vec2& arr_ele, const int& index);
	void drawCurve();
	void FetchData();
	void MoveCameraToNearestPoint();
	void ChangeInterval(APIInterval apiinterval);
};