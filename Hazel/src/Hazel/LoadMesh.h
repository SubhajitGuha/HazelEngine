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
	class Material;
	class LoadMesh
	{
	public:
		enum LoadType
		{
			IMPORT_MESH, LOAD_MESH
		};
		struct SubMesh
		{
			std::vector<glm::vec3> Vertices;
			std::vector<glm::vec3> Normal;
			std::vector<glm::vec3> Tangent;
			std::vector<glm::vec3> BiTangent;
			std::vector<glm::vec2> TexCoord;
			ref<VertexArray> VertexArray;
			uint64_t m_MaterialID;
			uint32_t numVertices;
		};
	public:
		LoadMesh();
		LoadMesh(const std::string& Path, LoadType type = LoadType::LOAD_MESH);
		~LoadMesh();
		void CreateLOD(const std::string& Path, LoadType type = LoadType::LOAD_MESH);
		LoadMesh* GetLOD(int lodIndex);

	public:
		std::string m_path;
		std::vector<SubMesh> m_subMeshes;		
		glm::mat4 GlobalTransform;
		uint64_t uuid;
	private:
		std::string extension = ".asset";
		std::string objectName;
		std::vector<LoadMesh*> m_LOD;
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
			VertexAttributes(const glm::vec4& Position, const glm::vec2& TextureCoordinate, const glm::vec3& normal = { 0,0,0 }, const glm::vec3& Tangent = { 0,0,0 }, const glm::vec3& BiNormal = { 0,0,0 })
			{
				this->Position = Position;
				this->TextureCoordinate = TextureCoordinate;
				Normal = normal;
				this->Tangent = Tangent;
				this->BiNormal = BiNormal;
			}
			VertexAttributes() = default;
			//may more ..uv coord , tangents , normals..
		};
	private:
		void LoadObj(const std::string& Path);
		void ProcessNode(aiNode* Node, const aiScene* scene);
		void ProcessMesh();
		void ProcessMaterials(const aiScene* scene);
		void CalculateTangent();
		void CreateStaticBuffers();
	};
}