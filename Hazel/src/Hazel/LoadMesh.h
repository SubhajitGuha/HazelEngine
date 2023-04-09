#pragma once
#include "Hazel.h"
#include <fstream>
#include <sstream>
#include <vector>

struct aiMesh;
struct aiNode;
struct aiScene;
namespace Hazel {
	class Texture2DArray;
	class LoadMesh
	{
	public:
		LoadMesh();
		LoadMesh(const std::string& Path);
		~LoadMesh();
		void LoadObj(const std::string& Path);
	public:
		std::vector<glm::vec3> Vertices;
		std::vector<glm::vec2> TexCoord;
		std::vector<glm::vec3> Normal;
		std::vector<glm::vec3> Tangent;
		std::vector<glm::vec3> BiTangent;
		std::vector<unsigned int> Vertex_Indices;
		std::vector<unsigned int> Material_Index;
		ref<Texture2DArray> Diffuse_Texture = nullptr;
		ref<Texture2DArray> Roughness_Texture = nullptr;
		ref<Texture2DArray> Normal_Texture = nullptr;

	private:
		std::vector<aiMesh*> m_Mesh;
	private:
		void ProcessNode(aiNode* Node, const aiScene* scene);
		void ProcessMesh();
		void ProcessMaterials(const aiScene* scene);
		void CalculateTangent();
	};
}