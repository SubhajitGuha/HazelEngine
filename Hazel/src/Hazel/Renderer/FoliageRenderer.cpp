#include "hzpch.h"
#include "FoliageRenderer.h"
#include "glad/glad.h"
#include "Hazel/Renderer/Shadows.h"
#include "Hazel/Renderer/Terrain.h"
#include "Hazel/ResourceManager.h"

namespace Hazel {
	float data[2048 * 2048] = { 0.0f };
	bool resetDensityMap = true;
	glm::vec3 Foliage::Showcase_camPosition = { 300,10,300 };
	glm::vec3 Foliage::Showcase_camRotation = { 0,0,0 };
	uint32_t Foliage::m_DensityMapID; //
	uint32_t Foliage::class_ID = 0;
	std::vector<Foliage*> Foliage::foliageObjects;

	Foliage::Foliage(LoadMesh* mesh, uint32_t numInstances, uint32_t coverageX, uint32_t coverageY, float cullDistance,
		bool canCastShadow, float LOD_Distance, bool bapplyGradientMask, bool benableWind)
		:m_foliageMesh(mesh), m_instanceCount(numInstances), m_cullDistance(cullDistance), 
		bCanCastShadow(canCastShadow), lod0Distance(LOD_Distance), applyGradientMask(bapplyGradientMask), enableWind(benableWind)
	{
		class_ID++;
		m_coverage = glm::ivec2(coverageX, coverageY);
		camera = new EditorCamera(16, 9);
		camera->SetCameraPosition(Showcase_camPosition);
		FoliageDensityMap = Texture2D::Create("Assets/Textures/PerlinTexture.png");
		FoliageDensityMap->Bind(FOLIAGE_DENSITY_TEXTURE_SLOT);

		cs_FrustumCullVote = Shader::Create("Assets/Shaders/cs_FrustumCullVote.glsl");
		cs_PrefixSum = Shader::Create("Assets/Shaders/cs_FrustumCullPrefixSum.glsl");
		cs_FrustumCullCompact = Shader::Create("Assets/Shaders/cs_FrustumCullCompact.glsl");
		cs_FoliageSpawn = Shader::Create("Assets/Shaders/cs_ProceduralFoliagePlacement.glsl");
		cs_GrassPlacement = Shader::Create("Assets/Shaders/cs_GrassPlacement.glsl");
		cs_createLod = Shader::Create("Assets/Shaders/cs_CreateLODs.glsl");
		cs_CopyIndirectBufferData = Shader::Create("Assets/Shaders/cs_CopyIndirectBufferData.glsl");
		cs_ResetDensityMap = Shader::Create("Assets/Shaders/cs_ResetDensityMap.glsl");

		blueNoiseTexture = Texture2D::Create("Assets/Textures/Blue_Noise.png");
		blueNoiseTexture->Bind(BLUE_NOISE_TEXTURE_SLOT);

		glGenTextures(1, &m_DensityMapID);
		glBindTexture(GL_TEXTURE_2D, m_DensityMapID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 2048, 2048, 0, GL_RED, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		foliage_positions.resize(numInstances,glm::vec2(0.0,0.0));
		foliageObjects.push_back(this);//this is a static vector that keeps track of all foliage objects created
	}

	Foliage::~Foliage()
	{
		//delete(m_foliageMesh);
	}
	void Foliage::addInstance(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0), pos) *
			glm::rotate(glm::mat4(1.0), glm::radians(90.0f), { 0,0,1 }) *
			glm::rotate(glm::mat4(1.0), glm::radians(rot.y), { 1,0,0 }) *
			//glm::rotate(glm::mat4(1.0), glm::radians(rot.z), { 0,0,1 }) *			
			glm::scale(glm::mat4(1.0), scale);

		m_foliageTransforms.push_back(transform);
	}

	void Foliage::RenderFoliage(Camera& cam)
	{
		float fov = cam.GetVerticalFOV();
		cam.SetVerticalFOV(fov * 2);
		if (!bHasSpawnned)
			SpawnFoliage();

		//Frustum cull and distance cull 
		for (int i = 0; i < m_instanceCount; i += 1024)
		{
			Vote(cam, i);
			Scan(i);
			Compact(i);
		}
		CreateLODs(cam);
		cam.SetVerticalFOV(fov);

		Renderer3D::BeginSceneFoliage(cam);
		//LOD 0
		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(0), ssbo_outTransformsLOD0);//render lod0 elements
		for (auto sub_mesh : m_foliageMesh->GetLOD(0)->m_subMeshes)
		{
			cs_CopyIndirectBufferData->Bind();
			cs_CopyIndirectBufferData->SetInt("VertexBufferSize", sub_mesh.numVertices);

			if (ssbo_indirectBuffer_LOD0 == -1)
			{
				glGenBuffers(1, &ssbo_indirectBuffer_LOD0);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD0);
				glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawArraysIndirectCommand), &indirectBuffer_LOD0, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD0);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			else
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD0);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD0);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicCounter_lod0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, atomicCounter_lod0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			Renderer3D::DrawFoliageInstanced(sub_mesh, Terrain::m_terrainModelMat, ssbo_indirectBuffer_LOD0, Terrain::time, applyGradientMask, enableWind);
		}
		//LOD 1
		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(1), ssbo_outTransformsLOD1);	//render lod1 elements
		for (auto sub_mesh : m_foliageMesh->GetLOD(1)->m_subMeshes)
		{
			cs_CopyIndirectBufferData->Bind();
			cs_CopyIndirectBufferData->SetInt("VertexBufferSize", sub_mesh.numVertices);
		
			if (ssbo_indirectBuffer_LOD1 == -1)
			{
				glGenBuffers(1, &ssbo_indirectBuffer_LOD1);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD1);
				glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawArraysIndirectCommand), &indirectBuffer_LOD1, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD1);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			else
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD1);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD1);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicCounter_lod1);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, atomicCounter_lod1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			Renderer3D::DrawFoliageInstanced(sub_mesh, Terrain::m_terrainModelMat, ssbo_indirectBuffer_LOD1, Terrain::time, applyGradientMask, enableWind);
		}
	}

	//void Foliage::RenderFoliage(Camera& cam)
	//{
	//	float fov = cam.GetVerticalFOV();
	//	cam.SetVerticalFOV(fov * 2);
	//	//choose LOD here;		
	//	SpawnFoliage(cam.GetCameraPosition());
	//
	//	//Frustum cull and distance cull 
	//	for (int i = 0; i < m_instanceCount; i += 1024)
	//	{
	//		Vote(cam, i);
	//		Scan(i);
	//		Compact(i);
	//	}
	//	CreateLODs(cam);
	//	cam.SetVerticalFOV(fov);
	//
	//	Renderer3D::BeginSceneFoliage(cam);
	//	//LOD 0
	//	Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(0), ssbo_outTransformsLOD0);//render lod0 elements
	//	for (auto sub_mesh : m_foliageMesh->GetLOD(0)->m_subMeshes)
	//	{
	//		cs_CopyIndirectBufferData->Bind();
	//		cs_CopyIndirectBufferData->SetInt("VertexBufferSize", sub_mesh.numVertices);
	//
	//		if (ssbo_indirectBuffer_LOD0 == -1)
	//		{
	//			glGenBuffers(1, &ssbo_indirectBuffer_LOD0);
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD0);
	//			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawArraysIndirectCommand), &indirectBuffer_LOD0, GL_DYNAMIC_DRAW);
	//			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD0);
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//		}
	//		else
	//		{
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD0);
	//			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD0);
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//		}
	//
	//		glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicCounter_lod0);
	//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, atomicCounter_lod0);
	//		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//
	//		glDispatchCompute(1, 1, 1);
	//		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//		Renderer3D::DrawFoliageInstanced(sub_mesh, Terrain::m_terrainModelMat, ssbo_indirectBuffer_LOD0, Terrain::time, applyGradientMask, enableWind);
	//	}
	//	//LOD 1
	//	Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(1), ssbo_outTransformsLOD1);	//render lod1 elements
	//	for (auto sub_mesh : m_foliageMesh->GetLOD(1)->m_subMeshes)
	//	{
	//		cs_CopyIndirectBufferData->Bind();
	//		cs_CopyIndirectBufferData->SetInt("VertexBufferSize", sub_mesh.numVertices);
	//
	//		if (ssbo_indirectBuffer_LOD1 == -1)
	//		{
	//			glGenBuffers(1, &ssbo_indirectBuffer_LOD1);
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD1);
	//			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawArraysIndirectCommand), &indirectBuffer_LOD1, GL_DYNAMIC_DRAW);
	//			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD1);
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//		}
	//		else
	//		{
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_indirectBuffer_LOD1);
	//			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_indirectBuffer_LOD1);
	//			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//		}
	//
	//		glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicCounter_lod1);
	//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, atomicCounter_lod1);
	//		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//
	//		glDispatchCompute(1, 1, 1);
	//		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//		Renderer3D::DrawFoliageInstanced(sub_mesh, Terrain::m_terrainModelMat, ssbo_indirectBuffer_LOD1, Terrain::time, applyGradientMask, enableWind);
	//	}
	//}
	void Foliage::RenderFoliage(ref<Shader>& shadow_shader)
	{
		//LOD 0
		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(0), ssbo_outTransformsLOD0);	//render lod0 elements
		for (auto sub_mesh : m_foliageMesh->GetLOD(0)->m_subMeshes)
		{
			ref<Material> material = ResourceManager::allMaterials[sub_mesh.m_MaterialID]; //get material from the resource manager
			material->Diffuse_Texture->Bind(ALBEDO_SLOT);
			shadow_shader->SetInt("u_Albedo", ALBEDO_SLOT); //alpha channel is being used
			RenderCommand::DrawArraysIndirect(*sub_mesh.VertexArray, ssbo_indirectBuffer_LOD0);			
		}
	}
	glm::mat4 Foliage::getTransform(int index)
	{
		if (index >= m_foliageTransforms.size() || index < 0) {
			HAZEL_CORE_ERROR("index out of bounds of foliage transform array");
			return glm::mat4(1.0);
		}
		return m_foliageTransforms[index];
	}

	void Foliage::SetFoliageDistributionParam(float _spacing, float _zoi, float _trunk_radius, float _predominanceVal)
	{
		spacing = _spacing;
		zoi = _zoi;
		trunk_radius = _trunk_radius;
		predominanceValue = _predominanceVal;
	}

	void Foliage::Vote(Camera& cam, int offset)
	{
		cs_FrustumCullVote->Bind();
		cs_FrustumCullVote->SetFloat3("camPos", cam.GetCameraPosition());
		cs_FrustumCullVote->SetMat4("u_ViewProjection", cam.GetProjectionView());
		cs_FrustumCullVote->SetInt("offset", offset);
		cs_FrustumCullVote->SetFloat("u_cullDistance", m_cullDistance);
		cs_FrustumCullVote->SetFloat3("aabb_min", m_foliageMesh->total_bounds.aabbMin);
		cs_FrustumCullVote->SetFloat3("aabb_max", m_foliageMesh->total_bounds.aabbMax);

		if (ssbo_voteIndices == -1)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * m_instanceCount, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicCounter_numInstances);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, atomicCounter_numInstances);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	void Foliage::Scan(int offset)
	{
		cs_PrefixSum->Bind();
		cs_PrefixSum->SetInt("stride", offset / 2); //must be half the original offset value because of the way the algo works
		if (ssbo_PrefixSum == -1)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_PrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_PrefixSum);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * m_instanceCount, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_PrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_totalPrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totalPrefixSum);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &totalSum, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_totalPrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_PrefixSum);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_PrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totalPrefixSum);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_totalPrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

	}
	void Foliage::Compact(int offset)
	{
		cs_FrustumCullCompact->Bind();
		cs_FrustumCullCompact->SetInt("offset", offset);
		if (ssbo_outTransforms == -1)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_PrefixSum);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_PrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_outTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransforms);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_outTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_voteIndices);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_PrefixSum);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_PrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_outTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	void Foliage::CreateLODs(Camera& cam)
	{
		cs_createLod->Bind();
		cs_createLod->SetFloat3("u_camPos", cam.GetCameraPosition());
		cs_createLod->SetMat4("u_ModelMat", Terrain::m_terrainModelMat);
		cs_createLod->SetFloat("u_LOD0Distance", lod0Distance);

		if (ssbo_outTransformsLOD0 == -1)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_outTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_outTransformsLOD0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransformsLOD0);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount*0.5, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_outTransformsLOD0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_outTransformsLOD1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransformsLOD1);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount * 0.5, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_outTransformsLOD1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &atomicCounter_lod0);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod0);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &x, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, atomicCounter_lod0);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

			glGenBuffers(1, &atomicCounter_lod1);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod1);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &x, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, atomicCounter_lod1);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

			//pass the prefix sum (total amount of foliage to be rendered after frustum culling)
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totalPrefixSum);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_totalPrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_outTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransformsLOD0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_outTransformsLOD0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransformsLOD1);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_outTransformsLOD1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod0);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, atomicCounter_lod0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod1);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, atomicCounter_lod1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			//pass the prefix sum (total amount of foliage to be rendered after frustum culling)
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totalPrefixSum);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_totalPrefixSum);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		if(m_instanceCount <64)
			glDispatchCompute(1, 1, 1);
		else
			glDispatchCompute((int)std::ceil(m_instanceCount/64.0), 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	void Foliage::GenerateFoliagePositions(Bounds& bounds)
	{
		//if (PDD_values.size() == 0)
			PDD_values = GeneratePoints(spacing, bounds, 10); //use poisson disk distribution to populate

		if (PDD_values.size() < m_instanceCount && total_instanceCount < m_instanceCount)
		{
			for (uint32_t i = 0; i < PDD_values.size(); i++)
			{
				if ((total_instanceCount + i) < m_instanceCount)
					foliage_positions[total_instanceCount + i] = glm::vec2(bounds.aabbMin.x, bounds.aabbMin.z) + PDD_values[i];
			}
			total_instanceCount += PDD_values.size();
		}
	}

	void Foliage::RemoveFoliagePositions(Bounds& bounds)
	{
		//delete positions which are not in between the bounds
		auto newEnd = std::remove_if(std::execution::par, foliage_positions.begin(), foliage_positions.begin() + total_instanceCount,
			[bounds](glm::vec2& cur_val) {return (cur_val.x >= bounds.aabbMin.x && cur_val.x <= bounds.aabbMax.x&&
				cur_val.y >= bounds.aabbMin.z && cur_val.y <= bounds.aabbMax.z); });
		//fill the remaining positions with zero
		//std::fill(std::execution::par, newEnd, foliage_positions.end(), glm::vec2(0.0, 0.0));
		total_instanceCount = newEnd - foliage_positions.begin(); // update the total instance count (true if PDD_values is constant size)
		
		//when a terrain chunk is deleted re-calculate the density map by drawing all foliage-objects again
		for (Foliage*& foliage : foliageObjects) 
		{
			foliage->bHasSpawnned = false; //respawn the foliage to update the buffers on the gpu side
			foliage->bResetGpuInstanceCount = true; //reset the instance counter on gpu
		}

		//this compute shader outputs a black density map (actually resetting it)
		//this works and performs much better than using "glTexSubImage2D"
		cs_ResetDensityMap->Bind();
		glBindImageTexture(0, m_DensityMapID, 0, 0, 0, GL_WRITE_ONLY, GL_R16F);
		glDispatchCompute(2048/32, 2048/32, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
	}

	void Foliage::SpawnFoliage()
	{
		bHasSpawnned = true;
		cs_FoliageSpawn->Bind();
		cs_FoliageSpawn->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		cs_FoliageSpawn->SetInt("u_DensityMap", FOLIAGE_DENSITY_TEXTURE_SLOT);
		cs_FoliageSpawn->SetFloat("u_HeightMapScale", Terrain::HeightScale);
		cs_FoliageSpawn->SetInt("u_nearestDistance", spacing);
		cs_FoliageSpawn->SetFloat("u_zoi", zoi);
		cs_FoliageSpawn->SetFloat("u_trunk_radius", trunk_radius);		
		cs_FoliageSpawn->SetInt("u_instanceCount", total_instanceCount);
		cs_FoliageSpawn->SetFloat("u_predominanceValue", predominanceValue);

		if (ssbo_inTransforms == -1)
		{
			glGenBuffers(1, &ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_foliagePos);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_foliagePos);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * m_instanceCount, &foliage_positions[0], GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_foliagePos);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &atomicCounter_numInstances);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_numInstances);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &x, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, atomicCounter_numInstances);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

			glBindImageTexture(3, m_DensityMapID, 0, 0, 0, GL_READ_WRITE, GL_R16F);
		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_foliagePos);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec2) * total_instanceCount, &foliage_positions[0]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_foliagePos);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_numInstances);
			if (bResetGpuInstanceCount) //only reset the total instance count when deleting quad-tree nodes in the terrain
			{
				glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t), &x);
				bResetGpuInstanceCount = false;
			}
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, atomicCounter_numInstances);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		
			glBindImageTexture(3, m_DensityMapID, 0, 0, 0, GL_READ_WRITE, GL_R16F);
		}

		glDispatchCompute(std::ceil(total_instanceCount / 1024.0), 1 , 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	void Foliage::SpawnFoliage(glm::vec3 playerPos)
	{
		glm::vec2 dist = glm::vec2(abs(playerPos.x - oldPlayerPos.x), abs(playerPos.z - oldPlayerPos.y));
		//HAZEL_CORE_ERROR("{}{}", "Distance is = ", dist);
		if ((dist.x > 100 || dist.y > 100))
			return;
		oldPlayerPos = glm::vec2(playerPos.x, playerPos.z);
		cs_GrassPlacement->Bind();
		cs_GrassPlacement->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		cs_GrassPlacement->SetInt("u_DensityMap", FOLIAGE_DENSITY_TEXTURE_SLOT);
		cs_GrassPlacement->SetInt("u_BlueNoise", BLUE_NOISE_TEXTURE_SLOT);
		cs_GrassPlacement->SetFloat3("u_PlayerPos", playerPos);
		cs_GrassPlacement->SetFloat("u_HeightMapScale", Terrain::HeightScale);
		cs_GrassPlacement->SetFloat("u_instanceCount", m_instanceCount);
		cs_GrassPlacement->SetFloat("u_spacing", m_Spacing);

		if (ssbo_inTransforms == -1)
		{
			glGenBuffers(1, &ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &atomicCounter_numInstances);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_numInstances);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &x, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, atomicCounter_numInstances);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_numInstances);			
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, atomicCounter_numInstances);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		}

		glDispatchCompute(m_coverage.x / 32, m_coverage.y / 32, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	auto IsValid = [&](glm::vec2 candidate, glm::vec2 sampleRegionSize, float cellSize, float radius, std::vector<glm::vec2> points, const std::vector<std::vector<int>>& grid)
	{
		if (candidate.x >= 0 && candidate.x < sampleRegionSize.x && candidate.y >= 0 && candidate.y < sampleRegionSize.y) {
			int cellX = (int)(candidate.x / cellSize);
			int cellY = (int)(candidate.y / cellSize);
			int searchStartX = glm::max(0, cellX - 2);
			int searchEndX = glm::min(cellX + 2, (int)grid[0].size() - 1);
			int searchStartY = glm::max(0, cellY - 2);
			int searchEndY = glm::min(cellY + 2, (int)grid.size() - 1);

			for (int x = searchStartX; x <= searchEndX; x++) {
				for (int y = searchStartY; y <= searchEndY; y++) {
					int pointIndex = grid[x][y] - 1;
					if (pointIndex != -1) {
						float sqrDst = glm::distance2(candidate, points[pointIndex]);
						if (sqrDst < radius * radius) {
							return false;
						}
					}
				}
			}
			return true;
		}
		return false;
	};

	std::vector<glm::vec2> Foliage::GeneratePoints(float radius, Bounds& bounds, int numSamplesBeforeRejection )
	{
		float cellSize = radius / 1.41421356237;
		auto extent = bounds.aabbMax - bounds.aabbMin;
		glm::vec2 sampleRegionSize = glm::vec2(extent.x, extent.z);//get the box size
		uint32_t seed = bounds.aabbMin.x + bounds.aabbMin.z + bounds.aabbMax.x + bounds.aabbMax.z;

		std::vector<std::vector<int>> grid((int)glm::ceil(sampleRegionSize.x / cellSize), std::vector<int>((int)glm::ceil(sampleRegionSize.y / cellSize)));
		std::vector<glm::vec2> points;
		std::vector<glm::vec2> spawnPoints;

		std::uniform_real_distribution<float> dist(0.0, 1.0);
		std::default_random_engine generator(seed + class_ID);

		spawnPoints.push_back({ sampleRegionSize.x / 2,sampleRegionSize.y / 2 });
		while (spawnPoints.size() > 0) {
			std::uniform_int_distribution<int> int_dist(0, spawnPoints.size()-1);

			int spawnIndex = int_dist(generator);
			glm::vec2 spawnCentre = spawnPoints[spawnIndex];
			bool candidateAccepted = false;

			for (int i = 0; i < numSamplesBeforeRejection; i++)
			{
				float angle = dist(generator) * glm::pi<float>() * 2;
				glm::vec2 dir = { glm::sin(angle), glm::cos(angle) };
				glm::vec2 candidate = spawnCentre + dir * (dist(generator) * radius + radius);
				if (IsValid(candidate, sampleRegionSize, cellSize, radius, points, grid)) {
					points.push_back(candidate); //add the point position in the world space
					spawnPoints.push_back(candidate);
					grid[(int)(candidate.x / cellSize)][(int)(candidate.y / cellSize)] = (int)points.size();
					candidateAccepted = true;
					break;
				}
			}
			if (!candidateAccepted) {
				spawnPoints.erase(spawnPoints.begin() + spawnIndex);
			}

		}

		return points;
	}

}
