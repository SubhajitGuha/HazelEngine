#include "MaterialEditor.h"
#include "Hazel/ResourceManager.h"

using namespace Hazel;

uint64_t MaterialEditor::cached_materialID;
MaterialEditor::MaterialEditor()
{
	cached_materialID = 0;
}

void MaterialEditor::OnImGuiRender()
{
	ref<Material>& cached_material = ResourceManager::allMaterials[cached_materialID];//get reference to the particular material of chached_materialID

	ImGui::Begin("Material Editor");
	if (cached_material)
	{
		ImGui::PushID(1); //to assign unique ids as texture paths can be blank all at the same time;
		if (ImGui::Button(cached_material->GetAlbedoPath().c_str(), { 120,60 }))
		{

		}
		ImGui::PopID();
		ImGui::PushID(2);
		if (ImGui::Button(cached_material->GetNormalPath().c_str(), { 120,60 }))
		{

		}
		ImGui::PopID();
		ImGui::PushID(3);
		if (ImGui::Button(cached_material->GetRoughnessPath().c_str(), { 120,60 }))
		{

		}
		ImGui::PopID();
	}
	glm::vec4 color = cached_material == nullptr? glm::vec4(1) : cached_material->GetColor();
	float roughness = cached_material == nullptr ? 1 : cached_material->GetRoughness();
	float metalness = cached_material == nullptr ? 1 : cached_material->GetMetalness();
	float normal = cached_material == nullptr ? 1 : cached_material->GetNormalStrength();
	ImGui::ColorEdit4("Color", glm::value_ptr(color));
	ImGui::DragFloat("Roughness", &roughness, 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat("Metallic", &metalness, 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat("Normal Strength", &normal, 0.1f, 0.0f, 1.0f);
	if (cached_material)
		cached_material->SetMaterialAttributes(color, roughness, metalness, normal);
	ImGui::End();
}
