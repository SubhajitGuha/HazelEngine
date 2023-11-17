#include "hzpch.h"
#include "FoliageRenderer.h"
#include "glad/glad.h"
#include "Hazel/Renderer/Shadows.h"
#include "Hazel/Renderer/Terrain.h"

namespace Hazel {
	glm::vec3 Foliage::Showcase_camPosition = { 300,10,300 };
	glm::vec3 Foliage::Showcase_camRotation = { 0,0,0 };

	std::vector<Foliage*> Foliage::foliageObjects;
	Foliage::Foliage(LoadMesh* mesh, uint32_t numInstances, uint32_t coverageX, uint32_t coverageY, float cullDistance, bool canCastShadow, float LOD_Distance)
		:m_foliageMesh(mesh), m_instanceCount(numInstances), m_cullDistance(cullDistance), bCanCastShadow(canCastShadow), lod0Distance(LOD_Distance)
	{
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
		cs_FoliageBuffersInit = Shader::Create("Assets/Shaders/cs_FoliageBufferInit.glsl");
		cs_createLod = Shader::Create("Assets/Shaders/cs_CreateLODs.glsl");


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

	auto CalculateFrustumPlanes = [&](Camera& cam)
	{

	};

	void Foliage::RenderFoliage(Camera& cam, float radius)
	{
		float fov = cam.GetVerticalFOV();
		cam.SetVerticalFOV(fov * 2);
		if (!bHasSpawnned)
			SpawnFoliage(cam.GetCameraPosition(), radius);

		//Frustum cull and distance cull 
		for (int i = 0; i < m_instanceCount; i += 1024)
		{
			Vote(cam, i);
			Scan(i);
			Compact(i);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totalPrefixSum);
		void* val = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		std::cout << "Tot foliage === " << *(int*)val << std::endl;
		renderedInstanceCount = *(int*)val;
		CreateLODs(cam);
		cam.SetVerticalFOV(fov);

		Renderer3D::BeginSceneFoliage(cam);
		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(0), ssbo_outTransformsLOD0);
		Renderer3D::DrawFoliageInstanced(*m_foliageMesh->GetLOD(0), Terrain::m_terrainModelMat, NumLOD0, { 1,1,1,1 }, Terrain::time);
	
		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(1), ssbo_outTransformsLOD1);
		Renderer3D::DrawFoliageInstanced(*m_foliageMesh->GetLOD(1), Terrain::m_terrainModelMat, NumLOD1, { 1,1,1,1 }, Terrain::time);
	}

	void Foliage::RenderFoliage(Camera& cam)
	{
		float fov = cam.GetVerticalFOV();
		cam.SetVerticalFOV(fov * 2);
		//choose LOD here;
		SpawnFoliage(cam.GetCameraPosition());

		//Frustum cull and distance cull 
		for (int i = 0; i < m_instanceCount; i += 1024)
		{
			Vote(cam, i);
			Scan(i);
			Compact(i);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totalPrefixSum);
		void* val = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		std::cout<< *(int*)val << std::endl;
		renderedInstanceCount = *(int*)val;
		CreateLODs(cam);
		cam.SetVerticalFOV(fov);

		Renderer3D::BeginSceneFoliage(cam);
		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(0), ssbo_outTransformsLOD0);	//render lod0 elements
		Renderer3D::DrawFoliageInstanced(*m_foliageMesh->GetLOD(0), Terrain::m_terrainModelMat, NumLOD0, { 1,1,1,1 }, Terrain::time);

		Renderer3D::InstancedFoliageData(*m_foliageMesh->GetLOD(1), ssbo_outTransformsLOD1);	//render lod1 elements
		Renderer3D::DrawFoliageInstanced(*m_foliageMesh->GetLOD(1), Terrain::m_terrainModelMat, NumLOD1, { 1,1,1,1 }, Terrain::time);
	}
	glm::mat4 Foliage::getTransform(int index)
	{
		if (index >= m_foliageTransforms.size() || index < 0) {
			HAZEL_CORE_ERROR("index out of bounds of foliage transform array");
			return glm::mat4(1.0);
		}
		return m_foliageTransforms[index];
	}

	void Foliage::Vote(Camera& cam, int offset)
	{
		cs_FrustumCullVote->Bind();
		cs_FrustumCullVote->SetFloat3("camPos", cam.GetCameraPosition());
		cs_FrustumCullVote->SetMat4("u_ViewProjection", cam.GetProjectionView());
		cs_FrustumCullVote->SetInt("offset", offset);
		cs_FrustumCullVote->SetFloat("u_cullDistance", m_cullDistance);

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
		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

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
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	void Foliage::CreateLODs(Camera& cam)
	{
		cs_createLod->Bind();
		cs_createLod->SetFloat3("u_camPos", cam.GetCameraPosition());
		cs_createLod->SetMat4("u_ModelMat", Terrain::m_terrainModelMat);
		cs_createLod->SetInt("u_InstanceCount", renderedInstanceCount);
		cs_createLod->SetFloat("u_LOD0Distance", lod0Distance);

		if (ssbo_outTransformsLOD0 == -1)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_outTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_outTransformsLOD0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransformsLOD0);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount*0.3, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_outTransformsLOD0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_outTransformsLOD1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransformsLOD1);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount * 0.7, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_outTransformsLOD1);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &atomicCounter_lod0);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod0);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &x, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, atomicCounter_lod0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &atomicCounter_lod1);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod1);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &x, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, atomicCounter_lod1);
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
		}

		if(renderedInstanceCount<64)
			glDispatchCompute(1, 1, 1);
		else
			glDispatchCompute((int)std::ceil(renderedInstanceCount/64.0), 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod0);
		NumLOD0 = *(uint32_t*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		std::cout<<"lod0 Count ==== "<< NumLOD0 << std::endl;

		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter_lod1);
		NumLOD1 = *(uint32_t*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		std::cout << "lod1 Count ==== " << NumLOD1 << std::endl;

	}
	
	void Foliage::SpawnFoliage(glm::vec3 playerPos, float radius)
	{
		bHasSpawnned = true;
		cs_FoliageSpawn->Bind();
		cs_FoliageSpawn->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		cs_FoliageSpawn->SetInt("u_DensityMap", FOLIAGE_DENSITY_TEXTURE_SLOT);
		cs_FoliageSpawn->SetFloat3("u_PlayerPos", playerPos);
		cs_FoliageSpawn->SetFloat("u_HeightMapScale", Terrain::HeightScale);


		auto m_foliagePos = GeneratePoints(radius, { m_coverage.x,m_coverage.y}, 10); //use poisson disk distribution to populate
		cs_FoliageSpawn->SetFloat("u_instanceCount", m_foliagePos.size());
		m_instanceCount = m_foliagePos.size();

		if (ssbo_inTransforms == -1)
		{
			glGenBuffers(1, &ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * m_instanceCount, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glGenBuffers(1, &ssbo_foliagePos);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_foliagePos);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * m_instanceCount, &m_foliagePos[0], GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_foliagePos);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_foliagePos);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_foliagePos);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		glDispatchCompute(m_coverage.x / 32, m_coverage.y / 32, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void Foliage::SpawnFoliage(glm::vec3 playerPos)
	{
		float dist = glm::distance(glm::vec2(playerPos.x, playerPos.z), oldPlayerPos);
		HAZEL_CORE_ERROR("{}{}", "Distance is = ", dist);
		if (dist < m_coverage.x-100)
			return;
		oldPlayerPos = glm::vec2(playerPos.x, playerPos.z);
		cs_GrassPlacement->Bind();
		cs_GrassPlacement->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		cs_GrassPlacement->SetInt("u_DensityMap", FOLIAGE_DENSITY_TEXTURE_SLOT);
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

		}
		else
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_inTransforms);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_inTransforms);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		glDispatchCompute(m_coverage.x / 32, m_coverage.y / 32, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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

	std::vector<glm::vec2> Foliage::GeneratePoints(float radius, glm::vec2 sampleRegionSize, int numSamplesBeforeRejection )
	{
		float cellSize = radius / 1.41421356237;
		std::vector<std::vector<int>> grid((int)glm::ceil(sampleRegionSize.x / cellSize), std::vector<int>((int)glm::ceil(sampleRegionSize.y / cellSize)));
		std::vector<glm::vec2> points;
		std::vector<glm::vec2> spawnPoints;

		std::uniform_real_distribution<float> dist(0, 1.0);
		std::default_random_engine generator;

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
					points.push_back(candidate);
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