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
		GlobalTransform = glm::mat4(1.0);
		m_path = Path;	
		if (m_LOD.size() == 0)
			m_LOD.push_back(this);
		LoadObj(Path);
	}
	LoadMesh::~LoadMesh()
	{
	}
	void LoadMesh::LoadObj(const std::string& Path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path,aiProcess_OptimizeGraph | aiProcess_FixInfacingNormals | aiProcess_SplitLargeMeshes | aiProcess_CalcTangentSpace );
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
	}

	void LoadMesh::CreateLOD(const std::string& Path)
	{
		m_LOD.push_back(new LoadMesh(Path));
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
			for (int k = 0; k < m_Mesh[i]->mNumVertices; k++) 
			{
				Material_Index.push_back(material_ind);
				
				aiVector3D aivertices = m_Mesh[i]->mVertices[k];
				glm::vec4 pos = GlobalTransform * glm::vec4(aivertices.x, aivertices.y, aivertices.z, 1.0);
				Vertices.push_back({ pos.x,pos.y,pos.z });

				if (m_Mesh[i]->HasNormals()) {
					aiVector3D ainormal = m_Mesh[i]->mNormals[k];
					glm::vec4 norm = GlobalTransform * glm::vec4(ainormal.x, ainormal.y, ainormal.z, 0.0);
					Normal.push_back({ norm.x,norm.y,norm.z });
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
					glm::vec4 tan = GlobalTransform * glm::vec4(tangent.x, tangent.y, tangent.z, 0.0);
					glm::vec4 bitan = GlobalTransform * glm::vec4(bitangent.x, bitangent.y, bitangent.z, 0.0);

					Tangent.push_back({ tan.x, tan.y, tan.z });
					BiTangent.push_back({ bitan.x,bitan.y,bitan.z });
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
	void LoadMesh::CreateStaticBuffers()
	{
		std::vector<VertexAttributes> buffer(Vertices.size());
		VertexArray = VertexArray::Create();

		for (int i = 0; i < Vertices.size(); i++)
		{
			glm::vec3 transformed_normals = (Normal[i]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
			glm::vec3 transformed_tangents = (Tangent[i]);
			glm::vec3 transformed_binormals = (BiTangent[i]);
			buffer[i] = (VertexAttributes(glm::vec4(Vertices[i], 1.0), TexCoord[i], transformed_normals, transformed_tangents, transformed_binormals, Material_Index[i]));
		}

		vb = VertexBuffer::Create(&buffer[0].Position.x,sizeof(VertexAttributes) * Vertices.size());

		bl = std::make_shared<BufferLayout>(); //buffer layout

		bl->push("position", DataType::Float4);
		bl->push("TexCoord", DataType::Float2);
		bl->push("Normal", DataType::Float3);
		bl->push("Tangent", DataType::Float3);
		bl->push("BiTangent", DataType::Float3);
		bl->push("Material_Index", DataType::Int);
		

		VertexArray->AddBuffer(bl, vb);
	}
}