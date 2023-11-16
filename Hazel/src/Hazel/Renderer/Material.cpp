#include "hzpch.h"
#include "Material.h"

namespace Hazel
{
	ref<Material> Material::m_material;
	std::unordered_map<uint64_t, ref<Material>> Material::allMaterials;

	ref<Material> Material::Create()
	{
		m_material = std::make_shared<Material>();
		return m_material;
	}
	Material::Material()
	{
		color = { 1,1,1 };
		roughness_multipler = 1;
		metallic_multipler = 0;
		normal_multipler = 1;
		emission = 1;

		UUID uuid;
		materialID = uuid;
	}
	void Material::SetTexturePaths(std::string& albedo_path, std::string& normal_path, std::string& roughness_path)
	{
		albedoPath = albedo_path;
		normalPath = normal_path;
		roughnessPath = roughness_path;

		CreateTextures();
	}
	void Material::SetMaterialAttributes(const glm::vec3& color, float roughness, float metalness, float normal_strength)
	{
		this->color = color;
		roughness_multipler = roughness;
		metallic_multipler = metalness;
		normal_multipler = normal_strength;
	}
	void Material::SerializeMaterial(const std::string& path)
	{
	}
	void Material::DeserializeMaterial()
	{
	}
	void Material::CreateTextures()
	{
		Diffuse_Texture = Texture2D::Create(albedoPath);
		Roughness_Texture = Texture2D::Create(roughnessPath);
		Normal_Texture = Texture2D::Create(normalPath);
	}
}