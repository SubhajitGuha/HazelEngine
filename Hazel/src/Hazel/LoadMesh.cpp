#include "hzpch.h"
#include "LoadMesh.h"
#include "Log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Hazel {
	LoadMesh::LoadMesh()
	{
	}
	LoadMesh::LoadMesh(const std::string& Path)
		:Vertices(0),Normal(0),TexCoord(0)
	{
		LoadObj(Path);
	}
	LoadMesh::~LoadMesh()
	{
	}
	void LoadMesh::LoadObj(const std::string& Path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path,aiProcess_FindDegenerates| aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes| aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_SplitLargeMeshes | aiProcess_CalcTangentSpace);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			HAZEL_CORE_ERROR("ERROR::ASSIMP::");
			HAZEL_CORE_ERROR(importer.GetErrorString());
			return;
		}

		ProcessMaterials(scene);
		ProcessNode(scene->mRootNode, scene);
		ProcessMesh();
	}

	void LoadMesh::ProcessNode(aiNode* Node, const aiScene* scene)
	{
		for (int i = 0; i < Node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[Node->mMeshes[i]];
			m_Mesh.push_back(mesh);
		}

		for (int i = 0; i < Node->mNumChildren; i++)
		{
			ProcessNode(Node->mChildren[i], scene);
		}
	}
	void LoadMesh::ProcessMesh()
	{
		for (int i = 0; i < m_Mesh.size(); i++)
		{
			unsigned int material_ind = m_Mesh[i]->mMaterialIndex;

			for (int k = 0; k < m_Mesh[i]->mNumVertices; k++) 
			{
				Material_Index.push_back(material_ind);
				
				aiVector3D aivertices = m_Mesh[i]->mVertices[k];
				Vertices.push_back({ aivertices.x,aivertices.y,aivertices.z });

				if (m_Mesh[i]->HasNormals()) {
					aiVector3D ainormal = m_Mesh[i]->mNormals[k];
					Normal.push_back({ ainormal.x,ainormal.y,ainormal.z });
				}

				glm::vec2 coord(0.0f);
				if (m_Mesh[i]->mTextureCoords[0])
				{
					coord.x = m_Mesh[i]->mTextureCoords[0][k].x;
					coord.y = m_Mesh[i]->mTextureCoords[0][k].y;

					TexCoord.push_back(coord);
				}
				else
					TexCoord.push_back(coord);

				if (m_Mesh[i]->HasTangentsAndBitangents())
				{
					aiVector3D tangent = m_Mesh[i]->mTangents[k];
					aiVector3D bitangent = m_Mesh[i]->mBitangents[k];
					Tangent.push_back({ tangent.x, tangent.y, tangent.z });
					BiTangent.push_back({ bitangent.x,bitangent.y,bitangent.z });
					//m_Mesh[i]->biTange
				}
				else
				{
					Tangent.push_back({ 0,0,0 });
					BiTangent.push_back({ 0,0,0 });
				}
			}
			for (int k = 0; k < m_Mesh[i]->mNumFaces; k++)
			{
				aiFace face = m_Mesh[i]->mFaces[k];
				for (int j = 0; j < face.mNumIndices; j++)
					Vertex_Indices.push_back(face.mIndices[j]);
			}
		}
	}
	void LoadMesh::ProcessMaterials(const aiScene* scene)//get all the materials in a scene
	{
		int NumMaterials = scene->mNumMaterials;

		std::string relative_path = "Assets/Textures/MeshTextures/";
		auto CreateTextureArray = [&](ref<Texture2DArray>& texture, aiTextureType type) 
		{
			std::vector<std::string> texture_path;
			for (int k = 0; k < NumMaterials; k++)
			{
				aiMaterial* material = scene->mMaterials[k];

				auto x = material->GetTextureCount(type);
				if (x > 0)
				{
					aiString str;
					material->GetTexture(type, 0, &str);
					std::string absolute_path = str.data;
					
					texture_path.push_back(relative_path + absolute_path.substr(absolute_path.find_last_of("\\")+1));
				}
			}
			ref<Texture2DArray> tex = Texture2DArray::Create(texture_path, NumMaterials);
			texture = tex;//load the diffuse textures
		};
		CreateTextureArray(Diffuse_Texture, aiTextureType_DIFFUSE);
		CreateTextureArray(Normal_Texture, aiTextureType_NORMALS);
		CreateTextureArray(Roughness_Texture, aiTextureType_SHININESS);

	}
	void LoadMesh::CalculateTangent()
	{
		glm::vec3 tangent = { 0.f,0.f,0.f };

	}
}