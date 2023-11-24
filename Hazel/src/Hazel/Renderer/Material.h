#pragma once
#include "Hazel.h"
#include "Hazel/UUID.h"

namespace Hazel
{
	class Material
	{
	public:
		static ref<Material> Create();
		Material();
		void SetTexturePaths(std::string& albedo_path, std::string& normal_path, std::string& roughness_path);
		void SetMaterialAttributes(const glm::vec4& color, float roughness, float metalness, float normal_strength);
		void SerializeMaterial(const std::string& path);
		void SetEmission(float emission) { this->emission = emission; }
		void SetShader(ref<Shader> shader) { RenderShader = shader; }
		inline ref<Shader> GetShader() { return RenderShader; }
		inline glm::vec4 GetColor() { return color; }
		inline float GetRoughness() { return roughness_multipler; }
		inline float GetMetalness() { return metallic_multipler; }
		inline float GetNormalStrength() { return normal_multipler; }
		inline float GetEmission() { return emission; }
		inline std::string GetAlbedoPath() { return albedoPath; }
		inline std::string GetNormalPath() { return normalPath; }
		inline std::string GetRoughnessPath() { return roughnessPath; }

		static void DeserializeMaterial(); //iterate through the solution directory and load the ".mat" files
	private:
		void CreateTextures();
	public:
		uint64_t materialID;
		std::string m_MaterialName;
		ref<Texture2D> Diffuse_Texture;
		ref<Texture2D> Roughness_Texture ;
		ref<Texture2D> Normal_Texture;
	private:
		static ref<Material> m_material;
		static uint32_t materialNameOffset;
		static std::string extension;
		glm::vec4 color;
		float metallic_multipler;
		float roughness_multipler;
		float normal_multipler;
		float emission;
		std::string albedoPath = "";
		std::string normalPath = "";
		std::string roughnessPath = "";
		ref<Shader> RenderShader;
	};
}