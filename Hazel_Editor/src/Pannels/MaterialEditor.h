#pragma once
#include "Hazel.h"
#include "Hazel/Renderer/Material.h"

using namespace Hazel;
class MaterialEditor
{
public:
	static uint64_t cached_materialID;
public:
	MaterialEditor();
	~MaterialEditor() = default;
	void OnImGuiRender();
private:
	std::filesystem::path m_filePath;
};

