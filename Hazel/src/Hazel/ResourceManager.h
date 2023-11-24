#pragma once
#include "Hazel/Core.h"
#include "Hazel/Renderer/Material.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/LoadMesh.h";

namespace Hazel
{
	class ResourceManager
	{
	public:
		ResourceManager() = default;
	public:
		static std::unordered_map<uint64_t, ref<Material>> allMaterials;
		static std::unordered_map<uint64_t, ref<Texture>> allTextures;
		static std::unordered_map<uint64_t, ref<LoadMesh>> allMeshes;
	};
}