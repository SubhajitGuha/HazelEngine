#define CURL_STATICLIB
#include "hzpch.h"
#include "DisplayNews.h"
#include "curl/curl.h"
#include "ImGuiInputText.h"

size_t write_data1(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}


News::News()
{
}

void News::ParseNewsData()
{
	news_param.clear();
	ImageCount = 0;

	std::ifstream file("news_data.json");
	Json::Value jsondata;
	Json::Reader jsonreader;

	if (jsonreader.parse(file, jsondata))
	{

		for(auto json_iterator = jsondata["feed"].begin() ; json_iterator!=jsondata["feed"].end() ; json_iterator++)
		{
			if (json_iterator.index() < NewsStart)
				continue;
			if (news_param.size() >= 4)//show 4 news at a time
				break;
			NewsParameters param;
			for (auto i = json_iterator->begin(); i != json_iterator->end(); i++) {
				HAZEL_CORE_INFO(i.name());
				//HAZEL_CORE_ERROR(i.memberName());
				if (i.name().compare("url") == 0)
					param.url = i->asString();
				if (i.name().compare("title") == 0)
					param.title = i->asString();
				if (i.name().compare("summary") == 0)
					param.summary = i->asString();
				if (i.name().compare("banner_image") == 0)
				{
					if (i->isNull() || i->asString() == "") {
						param.banner_image = "";
						image_thread = std::thread([=]() {SaveImage(param.banner_image); });
						image_thread.detach();
						param.texture = Texture2D::Create("Assets/Textures/Test.png");
					}
					else
					{
						param.banner_image = i->asString();
						image_thread = std::thread([=]() {SaveImage(param.banner_image); });
						image_thread.detach();

						//param.texture = Texture2D::Create("Assets/Textures/img_" + std::to_string(ImageCount - 1) + ".png");
					}
				}
			}
			news_param.push_back(param);
		}
		NewsStart += 4;//show 4 news at a time
	}
	file.close();
}


void News::OnAttach()
{
	auto GetNews = [&]()
	{
		CURL* curl;
		CURLcode res;
		FILE* fp;
		std::string api = "https://www.alphavantage.co/query?function=NEWS_SENTIMENT&"+tickers+"&time_from=20220410T0130&limit=200&apikey=RPX72XW6HBJ7J59R";
		char outfilename[FILENAME_MAX] = "news_data.json";
		curl = curl_easy_init();
		if (curl) {
			fp = fopen(outfilename, "wb");
			if (fp == NULL)
				HAZEL_CORE_ERROR("File not found");
			curl_easy_setopt(curl, CURLOPT_URL, api.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data1);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			if (CURLE_OK != res) {
				std::cerr << "CURL error: " << res << '\n';
				HAZEL_CORE_ERROR("CURL_ERROR");
			}
			fclose(fp);
		}
		curl_global_cleanup();
		//create file object)//create file object
		ParseNewsData();
	};

	GetNews();
}

void News::OnImGuiRender()
{
	ImGui::Begin("News");
	{
		ImGui::InputText("Tickers", &tickers);
		if (ImGui::Button("APPLY"))
			OnAttach();
			for (int i = 0; i < news_param.size(); i++) 
			{
				//if (image_thread.joinable()) {
				if (news_param[i].texture != nullptr) {
					auto tex = news_param[i].texture;
					float scale = 1;
					if (tex->GetWidth() > 250)
						scale = 250.0 / tex->GetWidth();
					if (tex->GetHeight() > 250)
						scale = 250.0 / tex->GetHeight();
					float AspectRatio = tex->GetWidth() / tex->GetHeight();
					ImGui::Image(reinterpret_cast<void*>(news_param[i].texture->GetID()), { tex->GetWidth()*scale,tex->GetHeight()*scale }, { 0,1 }, { 1,0 });
				}

				ImGui::PushFont(ImGuiLayer::Font);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
				ImGui::TextWrapped(news_param[i].title.c_str());
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::Text("\n");
				ImGui::TextWrapped(news_param[i].summary.c_str());
				ImGui::Text("\n");

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.0f, 1.0f));
				ImGui::TextWrapped(news_param[i].url.c_str());
				ImGui::PopStyleColor();

				std::wstring Wstr = std::wstring(news_param[i].url.begin(), news_param[i].url.end());
				auto url = Wstr.c_str();
				if (ImGui::IsItemClicked())
					ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWDEFAULT);//system call to open web browser

				ImGui::Text("\n\n");
			}
			if (ImGui::Button("Load More")) {
				ParseNewsData();
			}
	}
	ImGui::End();
}

void News::OnUpdate()
{
	for (int i = 0; i < news_param.size(); i++)
	{
		std::string path = "Assets/Textures/img_" + std::to_string(i) + ".png";
		if (news_param[i].texture == nullptr)
		{
			if (Texture2D::ValidateTexture(path))
				news_param[i].texture = Texture2D::Create(path);
			else
				news_param[i].texture = Texture2D::Create("Assets/Textures/Test.png");
		}
		std::remove(path.c_str());
	}
}


void News::SaveImage(const std::string& img_url)
{
	std::lock_guard<std::mutex> lock(m1);

	CURL* curl;
	CURLcode res;
	FILE* fp;
	std::string outfilename = "Assets/Textures/img_" + std::to_string(ImageCount) + ".png";
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename.c_str(), "wb");
		curl_easy_setopt(curl, CURLOPT_URL, img_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data1);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (CURLE_OK != res) {
			std::cerr << "CURL error: " << res << '\n';
		}
		else {
			ImageCount++;
		}
		if (fp == NULL)
			HAZEL_CORE_ERROR("No file found");
		fclose(fp);
	}
	curl_global_cleanup();
			
	//free(fp);
}
