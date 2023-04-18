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
		ref<VertexArray> VertexArray;

	private:
		ref<BufferLayout> bl;
		ref<VertexBuffer> vb;
		ref<IndexBuffer> ib;
		std::vector<aiMesh*> m_Mesh;
		struct VertexAttributes {
			//glm::vec3 Position;
			glm::vec4 Position;
			glm::vec2 TextureCoordinate;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 BiNormal;
			unsigned int Material_index = 0;//serves as an index to the array of texture slot which is passed as an uniform in init()
			VertexAttributes(const glm::vec4& Position, const glm::vec2& TextureCoordinate, const glm::vec3& normal = { 0,0,0 }, const glm::vec3& Tangent = { 0,0,0 }, const glm::vec3& BiNormal = { 0,0,0 }, unsigned int Material_index = 0)
			{
				this->Position = Position;
				this->TextureCoordinate = TextureCoordinate;
				this->Material_index = Material_index;
				Normal = normal;
				this->Tangent = Tangent;
				this->BiNormal = BiNormal;
			}
			VertexAttributes() = default;
			//may more ..uv coord , tangents , normals..
		};
	private:
		void ProcessNode(aiNode* Node, const aiScene* scene);
		void ProcessMesh();
		void ProcessMaterials(const aiScene* scene);
		void CalculateTangent();
		void CreateStaticBuffers();
	};
}