#include "hzpch.h"
#include "ResourceManager.h"

namespace Hazel
{
	std::unordered_map<uint64_t, ref<Material>> ResourceManager::allMaterials;
	std::unordered_map<uint64_t, ref<Texture>> ResourceManager::allTextures;
	std::unordered_map<uint64_t, ref<LoadMesh>> ResourceManager::allMeshes;
}