#pragma once
#include "Hazel.h"
#include "Hazel/Renderer/Material.h"

using namespace Hazel;
class MaterialEditor
{
public:
	static uint64_t cached_materialID;
	static std::string cached_texturePath;
public:
	MaterialEditor();
	~MaterialEditor() = default;
	void OnImGuiRender();
private:
	std::filesystem::path m_filePath;
};

