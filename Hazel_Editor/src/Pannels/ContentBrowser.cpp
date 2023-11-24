#include "hzpch.h"
#include "ContentBrowser.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/ResourceManager.h"
#include "MaterialEditor.h"

using namespace Hazel;
namespace FileSystem = std::filesystem;
constexpr char* dir = "Assets";

ContentBrowser::ContentBrowser()
{
	m_filePath = dir;
}

void ContentBrowser::OnImGuiRender()
{
	ImGui::Begin("Content Browser");
	if (ImGui::Button("<-", { 20,20 }))//back button
	{
		if (m_filePath.has_parent_path())
			m_filePath = m_filePath.parent_path();
	}
	for (const FileSystem::directory_entry& p : FileSystem::directory_iterator(m_filePath))//iterate throughr all the files
	{
		std::string filename = p.path().filename().string();
		if (p.is_directory()) {
			if (ImGui::Button(filename.c_str()))
			{
				m_filePath = m_filePath / p.path().filename();
			}
		}
		else {
			if (p.path().extension().string() == ".mat")
			{
				SceneSerializer ser;
				uint64_t materialID = ser.DeSerializeAndGetMaterialID(p.path().string()); //Get material Instance			
				if (ImGui::Button(filename.c_str()))
				{
					MaterialEditor::cached_materialID = materialID;
				}
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("Material payload", &ResourceManager::allMaterials[materialID], sizeof(filename));
					ImGui::Text(filename.c_str());
					ImGui::EndDragDropSource();
				}
				
			}
			//if (ImGui::BeginDragDropTarget())
			//{
			//	if (const ImGuiPayload* val = ImGui::AcceptDragDropPayload("Material payload"))
			//	{
			//		std::string path = *(const std::string*)val->Data;
			//		filename = path;
			//	}
			//	ImGui::EndDragDropTarget();
			//}
			//ImGui::Text(filename.c_str());
		}
	}
	ImGui::End();
}
