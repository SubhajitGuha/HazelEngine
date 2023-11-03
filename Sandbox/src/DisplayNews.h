#pragma once
#include "Hazel.h"
#include "json/json.h"

using namespace Hazel;
struct NewsParameters {
	std::string summary = "",
		title = "", url = "", banner_image = "";
	ref<Texture2D > texture;
};

class News
{
public:
	News();
	void OnAttach();
	void OnImGuiRender();
	void OnUpdate();
	void SaveImage(const std::string& img_url);
private:
	std::vector<NewsParameters> news_param;
	int NewsStart = 0;
	int ImageCount = 0;
	std::thread image_thread;
	std::mutex m1;
	std::string tickers = "";
	void ParseNewsData();
	//std::string NewsData;
};