#include "hzpch.h"
#include "ContentBrowser.h"
#include "Hazel/Scene/SceneSerializer.h"

using namespace Hazel;
namespace FileSystem = std::filesystem;
constexpr char* dir = "Assets";

ContentBrowser::ContentBrowser()
{
	m_filePath = dir;
}

void ContentBrowser::Context(const ref<Scene>& context)
{
	m_scene = std::move(context);
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
			ImGui::Button(filename.c_str());
			if (ImGui::BeginDragDropSource() && p.path().extension().string() == ".mat")
			{
				SceneSerializer ser;
				auto material = ser.DeSerializeAndGetMaterial(p.path().string()); //Get material Instance			

				ImGui::SetDragDropPayload("Material payload", &material, sizeof(filename));
				ImGui::Text(filename.c_str());
				ImGui::EndDragDropSource();
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
