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
		:vertices(0),Normal(0),TexCoord(0)
	{
		LoadObj(Path);
	}
	LoadMesh::~LoadMesh()
	{
	}
	void LoadMesh::LoadObj(const std::string& Path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path, aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_SplitLargeMeshes);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			HAZEL_CORE_ERROR("ERROR::ASSIMP::");
			HAZEL_CORE_ERROR(importer.GetErrorString());
			return;
		}
		ProcessNode(scene->mRootNode, scene);
		ProcessMesh();
	}

	void LoadMesh::ProcessNode(aiNode* Node, const aiScene* scene)
	{
		for (int i = 0; i < Node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[Node->mMeshes[i]];
			m_Mesh.push_back(mesh);
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				aiTextureType type = aiTextureType_BASE_COLOR;
				
				auto x = material->GetTextureCount(type);
				for (unsigned int j = 0; j < x; j++)
				{
					std::string str;
					material->GetTexture(type, j, (aiString*)&str);
					Diffuse_Texture.push_back(str);
				}
			}
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
			for (int k = 0; k < m_Mesh[i]->mNumVertices; k++) 
			{
				aiVector3D aivertices = m_Mesh[i]->mVertices[k];
				vertices.push_back({ aivertices.x,aivertices.y,aivertices.z });

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
			}
			for (int k = 0; k < m_Mesh[i]->mNumFaces; k++)
			{
				aiFace face = m_Mesh[i]->mFaces[k];
				for (int j = 0; j < face.mNumIndices; j++)
					Vertex_Indices.push_back(face.mIndices[j]);
			}
		}
	}
}