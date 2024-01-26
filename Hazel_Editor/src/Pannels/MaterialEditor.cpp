#include "MaterialEditor.h"
#include "Hazel/ResourceManager.h"
#include "Hazel/Scene/SceneSerializer.h"
using namespace Hazel;

uint64_t MaterialEditor::cached_materialID;
std::string MaterialEditor::cached_texturePath;
MaterialEditor::MaterialEditor()
{
	cached_materialID = 0;
	cached_texturePath = "";
}

void MaterialEditor::OnImGuiRender()
{
	ref<Material>& cached_material = ResourceManager::allMaterials[cached_materialID];//get reference to the particular material of chached_materialID

	auto HoverName = [](const std::string& name) 
	{
		if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(name.c_str());
		ImGui::EndTooltip();
	}};

	ImGui::Begin("Material Editor");
	if (cached_material)
	{
		ImGui::Text(cached_material->m_MaterialName.c_str());
		std::string albedo_path = cached_material->GetAlbedoPath(), normal_path = cached_material->GetNormalPath(), roughness_path = cached_material->GetRoughnessPath();
		ImGui::PushID(1); //to assign unique ids as texture paths can be blank all at the same time;
		ImGui::Button(cached_material->GetAlbedoPath().c_str(), { 120,60 });
		HoverName(cached_material->GetAlbedoPath()); //hover the path of the asset over the screen
		if (ImGui::BeginDragDropTarget())//drag drop diffuse texture from content browser
		{
			if (const ImGuiPayload* val = ImGui::AcceptDragDropPayload("Texture payload"))
			{
				albedo_path = *(std::string*)val->Data;//get the texture path which ImGui owns
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();
		ImGui::PushID(2);
		ImGui::Button(cached_material->GetNormalPath().c_str(), { 120,60 });
		HoverName(cached_material->GetNormalPath());//hover the path of the asset over the screen
		if (ImGui::BeginDragDropTarget())//drag drop texture from content browser
		{
			if (const ImGuiPayload* val = ImGui::AcceptDragDropPayload("Texture payload"))
			{
				normal_path = *(std::string*)val->Data;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();
		ImGui::PushID(3);
		ImGui::Button(cached_material->GetRoughnessPath().c_str(), { 120,60 });
		HoverName(cached_material->GetRoughnessPath());
		if (ImGui::BeginDragDropTarget())//drag drop texture from content browser
		{
			if (const ImGuiPayload* val = ImGui::AcceptDragDropPayload("Texture payload"))
			{
				roughness_path = *(std::string*)val->Data;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();
		cached_material->SetTexturePaths(albedo_path, normal_path, roughness_path); //problem area should not be hapenning every frame

		glm::vec4 color = cached_material->GetColor();
		float roughness = cached_material->GetRoughness();
		float metalness = cached_material->GetMetalness();
		float normal = cached_material->GetNormalStrength();
		ImGui::ColorEdit4("Color", glm::value_ptr(color));
		ImGui::DragFloat("Roughness", &roughness, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("Metallic", &metalness, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("Normal Strength", &normal, 0.1f, 0.0f, 1.0f);
		cached_material->SetMaterialAttributes(color, roughness, metalness, normal);
		if (ImGui::Button("Save Material"))
		{
			//save the material at the default location
			ResourceManager::allMaterials[cached_materialID]->SerializeMaterial("", ResourceManager::allMaterials[cached_materialID]->m_MaterialName);
		}
	}
	ImGui::End();
}
