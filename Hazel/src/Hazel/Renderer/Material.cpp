#include "hzpch.h"
#include "Material.h"
#include "Hazel/Scene/SceneSerializer.h"

namespace Hazel
{
	ref<Material> Material::m_material;
	std::unordered_map<uint64_t, ref<Material>> Material::allMaterials;
	uint32_t Material::materialNameOffset = 0;
	std::string Material::extension = ".mat";

	ref<Material> Material::Create()
	{
		m_material = std::make_shared<Material>();
		return m_material;
	}
	Material::Material()
	{
		materialNameOffset++; //increment when a material is created
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
		if (path == "")
		{
			std::filesystem::path materialpath("Assets/Materials");
			std::filesystem::create_directory(materialpath);
			std::string material_name = "Material" + std::to_string(materialNameOffset) + extension;
			std::string MatPath = (materialpath / std::filesystem::path(material_name)).string();
			SceneSerializer serialize;
			serialize.SerializeMaterial(MatPath, *this);
		}
		else
		{
			//TODO custom path
		}
	}
	void Material::DeserializeMaterial()
	{
		//iterate through the directory and load the .mat files
		std::filesystem::path materialpath("Assets/Materials"); //needs to be changed as we should have the ability to iterate the entire solution
		for (auto& p : std::filesystem::directory_iterator(materialpath))
		{
			if (p.path().extension().string() == extension)
			{
				std::string file_name = (p.path().parent_path() / p.path().filename()).string();
				ref<Material> mat = Create();
				SceneSerializer deserialize;
				deserialize.DeSerializeMaterial(file_name, *mat.get());
				allMaterials[mat->materialID] = mat;
			}
		}
	}
	void Material::CreateTextures()
	{
		Diffuse_Texture = Texture2D::Create(albedoPath);
		Roughness_Texture = Texture2D::Create(roughnessPath);
		Normal_Texture = Texture2D::Create(normalPath);
	}
}