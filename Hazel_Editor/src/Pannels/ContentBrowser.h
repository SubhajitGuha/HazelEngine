#pragma once
#include "Hazel.h"
#include <filesystem>

using namespace Hazel;

class ContentBrowser
{
public:
	ContentBrowser();
	~ContentBrowser() = default;
	void OnImGuiRender();
private:
	ref<Scene> m_scene;
	std::filesystem::path m_filePath;
};