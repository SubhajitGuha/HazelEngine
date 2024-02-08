#include "hzpch.h"
#include "LoadMesh.h"
#include "Log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Hazel/Renderer/Material.h"
#include "Hazel/UUID.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/ResourceManager.h"

namespace Hazel 
{
	LoadMesh::LoadMesh()
	{
	}
	LoadMesh::LoadMesh(const std::string& Path, LoadType type)
	{
		GlobalTransform = glm::mat4(1.0);
		std::filesystem::path mesh_path(Path);
		objectName = mesh_path.stem().string();
		m_path = (mesh_path.parent_path() / mesh_path.stem()).string() + extension; //temporary

		if (m_LOD.size() == 0)
			m_LOD.push_back(this);

		//if (type == IMPORT_MESH)
		{
			LoadObj(Path);
		}
		//else if (type == LOAD_MESH)
		//{
		//	SceneSerializer deserialize;
		//	deserialize.DeSerializeMesh(m_path, *this);
		//	uuid = UUID(Path); //only create uuid for engine compatible mesh
		//	//ResourceManager::allMeshes[uuid] = 
		//	CreateStaticBuffers();
		//}

	}
	LoadMesh::~LoadMesh()
	{
	}
	void LoadMesh::LoadObj(const std::string& Path)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(Path, aiProcess_OptimizeGraph | aiProcess_FixInfacingNormals | aiProcess_SplitLargeMeshes | aiProcess_CalcTangentSpace );
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			HAZEL_CORE_ERROR("ERROR::ASSIMP::");
			HAZEL_CORE_ERROR(importer.GetErrorString());
			return;
		}

		ProcessMaterials(scene);
		ProcessNode(scene->mRootNode, scene);
		ProcessMesh();
		CreateStaticBuffers();

		m_Mesh.clear();
		m_Mesh.shrink_to_fit();
	}

	void LoadMesh::CreateLOD(const std::string& Path, LoadType type)
	{
		m_LOD.push_back(new LoadMesh(Path,type));
	}

	LoadMesh* LoadMesh::GetLOD(int lodIndex)
	{
		if (lodIndex < m_LOD.size() && lodIndex >=0)
			return m_LOD[lodIndex];
		else
			return m_LOD[m_LOD.size()-1]; //if lod not available give the highest LOD
	}

	auto AssimpToGlmMatrix = [&](const aiMatrix4x4& from) {
		glm::mat4 to;
		to[0][0] = from.a1; to[0][1] = from.a2; to[0][2] = from.a3; to[0][3] = from.a4;
		to[1][0] = from.b1; to[1][1] = from.b2; to[1][2] = from.b3; to[1][3] = from.b4;
		to[2][0] = from.c1; to[2][1] = from.c2; to[2][2] = from.c3; to[2][3] = from.c4;
		to[3][0] = from.d1; to[3][1] = from.d2; to[3][2] = from.d3; to[3][3] = from.d4;

		return std::move(to);
	};

	void LoadMesh::ProcessNode(aiNode* Node, const aiScene* scene)
	{
		GlobalTransform *= AssimpToGlmMatrix(Node->mTransformation);
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
			m_subMeshes[material_ind].numVertices = m_Mesh[i]->mNumVertices;

			for (int k = 0; k < m_Mesh[i]->mNumVertices; k++) 
			{				
				aiVector3D aivertices = m_Mesh[i]->mVertices[k];
				glm::vec4 pos = GlobalTransform * glm::vec4(aivertices.x, aivertices.y, aivertices.z, 1.0);				
				m_subMeshes[material_ind].mesh_bounds.Union(glm::vec3(pos.x, pos.y, pos.z));
				m_subMeshes[material_ind].Vertices.push_back({ pos.x,pos.y,pos.z });

				if (m_Mesh[i]->HasNormals()) {
					aiVector3D ainormal = m_Mesh[i]->mNormals[k];
					glm::vec4 norm = GlobalTransform * glm::vec4(ainormal.x, ainormal.y, ainormal.z, 0.0);
					m_subMeshes[material_ind].Normal.push_back({ norm.x,norm.y,norm.z });
				}

				glm::vec2 coord(0.0f);
				if (m_Mesh[i]->mTextureCoords[0])
				{
					coord.x = m_Mesh[i]->mTextureCoords[0][k].x;
					coord.y = m_Mesh[i]->mTextureCoords[0][k].y;

					m_subMeshes[material_ind].TexCoord.push_back(coord);
				}
				else
					m_subMeshes[material_ind].TexCoord.push_back(coord);

				if (m_Mesh[i]->HasTangentsAndBitangents())
				{
					aiVector3D tangent = m_Mesh[i]->mTangents[k];
					aiVector3D bitangent = m_Mesh[i]->mBitangents[k];
					glm::vec4 tan = GlobalTransform * glm::vec4(tangent.x, tangent.y, tangent.z, 0.0);
					glm::vec4 bitan = GlobalTransform * glm::vec4(bitangent.x, bitangent.y, bitangent.z, 0.0);

					m_subMeshes[material_ind].Tangent.push_back({ tan.x, tan.y, tan.z });
					m_subMeshes[material_ind].BiTangent.push_back({ bitan.x,bitan.y,bitan.z });
					//m_Mesh[i]->biTange
				}
				else
				{
					m_subMeshes[material_ind].Tangent.push_back({ 0,0,0 });
					m_subMeshes[material_ind].BiTangent.push_back({ 0,0,0 });
				}
			}
			//for (int k = 0; k < m_Mesh[i]->mNumFaces; k++)
			//{
			//	aiFace face = m_Mesh[i]->mFaces[k];
			//	for (int j = 0; j < face.mNumIndices; j++)
			//		Vertex_Indices.push_back(face.mIndices[j]);
			//}
		}
	}
	void LoadMesh::ProcessMaterials(const aiScene* scene)//get all the materials in a scene
	{
		int NumMaterials = scene->mNumMaterials;
		m_subMeshes.resize(NumMaterials);

		std::string relative_path = "Assets/Textures/MeshTextures/";
		auto GetTexturePath = [&](aiMaterial*& material,aiTextureType type)
		{
			auto x = material->GetTextureCount(type);
			if (x > 0)
			{
				aiString str;
				material->GetTexture(type, 0, &str);
				std::string absolute_path = str.data;
				
				return (relative_path + absolute_path.substr(absolute_path.find_last_of("\\")+1));
			}
			return std::string("");
		};
		for (int i = 0; i < NumMaterials; i++)
		{
			aiMaterial* scene_material = scene->mMaterials[i];
			std::string materialName = objectName + std::string("_") + std::string(scene_material->GetName().C_Str());
			ref<Material> material = Material::Create(materialName, ""); //create a material in the default storage directory
			m_subMeshes[i].m_MaterialID = material->materialID;

			//if material cannot be found then create and serialize the material
			if (ResourceManager::allMaterials.find(material->materialID) == ResourceManager::allMaterials.end())			
			{
				std::string diffuse_path = GetTexturePath(scene_material, aiTextureType_DIFFUSE);
				std::string normal_path = GetTexturePath(scene_material, aiTextureType_NORMALS);
				std::string roughness_path = GetTexturePath(scene_material, aiTextureType_SHININESS);

				material->SetTexturePaths(diffuse_path, normal_path, roughness_path);
				material->SerializeMaterial("", materialName); //save the material
			}
		}
	}
	void LoadMesh::CalculateTangent()
	{
		glm::vec3 tangent = { 0.f,0.f,0.f };

	}
	void LoadMesh::CreateStaticBuffers()
	{
		for (int k = 0; k < m_subMeshes.size(); k++)
		{
			std::vector<VertexAttributes> buffer(m_subMeshes[k].Vertices.size());
			m_subMeshes[k].VertexArray = VertexArray::Create();

			for (int i = 0; i < m_subMeshes[k].Vertices.size(); i++)
			{
				glm::vec3 transformed_normals = (m_subMeshes[k].Normal[i]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
				glm::vec3 transformed_tangents = (m_subMeshes[k].Tangent[i]);
				glm::vec3 transformed_binormals = (m_subMeshes[k].BiTangent[i]);
				buffer[i] = (VertexAttributes(glm::vec4(m_subMeshes[k].Vertices[i], 1.0), m_subMeshes[k].TexCoord[i], transformed_normals, transformed_tangents, transformed_binormals));
			}

			vb = VertexBuffer::Create(&buffer[0].Position.x, sizeof(VertexAttributes) * m_subMeshes[k].Vertices.size());

			bl = std::make_shared<BufferLayout>(); //buffer layout

			bl->push("position", DataType::Float4);
			bl->push("TexCoord", DataType::Float2);
			bl->push("Normal", DataType::Float3);
			bl->push("Tangent", DataType::Float3);
			bl->push("BiTangent", DataType::Float3);

			m_subMeshes[k].VertexArray->AddBuffer(bl, vb);
		}
	}
}