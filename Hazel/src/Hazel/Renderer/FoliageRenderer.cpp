#include "hzpch.h"
#include "FoliageRenderer.h"
#include "glad/glad.h"
#include "Hazel/Renderer/Terrain.h"

namespace Hazel {
	glm::vec3 Foliage::Showcase_camPosition = { 300,10,300 };
	glm::vec3 Foliage::Showcase_camRotation = { 0,0,0 };

	Foliage::Foliage(LoadMesh* mesh, uint32_t numInstances, uint32_t coverageX, uint32_t coverageY, float cullDistance)
		:m_foliageMesh(mesh), m_instanceCount(numInstances), m_cullDistance(cullDistance)
	{
		m_coverage = glm::ivec2(coverageX, coverageY);
		camera = new EditorCamera(16, 9);
		camera->SetCameraPosition(Showcase_camPosition);
		FoliageDensityMap = Texture2D::Create("Assets/Textures/PerlinTexture.png");
		FoliageDensityMap->Bind(FOLIAGE_DENSITY_TEXTURE_SLOT);

		cs_FrustumCull = Shader::Create("Assets/Shaders/cs_FrustumCullVote.glsl");
		cs_PrefixSum = Shader::Create("Assets/Shaders/cs_FrustumCullPrefixSum.glsl");
		cs_FrustumCullCompact = Shader::Create("Assets/Shaders/cs_FrustumCullCompact.glsl");
		cs_FoliageSpawn = Shader::Create("Assets/Shaders/cs_ProceduralFoliagePlacement.glsl");
		cs_GrassPlacement = Shader::Create("Assets/Shaders/cs_GrassPlacement.glsl");
		cs_FoliageBuffersInit = Shader::Create("Assets/Shaders/cs_FoliageBufferInit.glsl");
		//m_foliageSpawnTransforms.resize(m_instanceCount,glm::mat4(1.0f));
		//Renderer3D::AllocateInstancedFoliageData(*m_foliageMesh, m_instanceCount, m_bufferID);
	}

	Foliage::~Foliage()
	{
		delete(m_foliageMesh);
	}
	void Foliage::addInstance(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0), pos) *
			glm::rotate(glm::mat4(1.0), glm::radians(90.0f), { 0,0,1 }) *
			glm::rotate(glm::mat4(1.0), glm::radians(rot.y), { 1,0,0 }) *
			//glm::rotate(glm::mat4(1.0), glm::radians(rot.z), { 0,0,1 }) *			
			glm::scale(glm::mat4(1.0), scale);

		m_foliageTransforms.push_back(transform);

		//procedural generation
	}

	auto CalculateFrustumPlanes = [&](Camera& cam)
	{

	};

	void Foliage::RenderFoliage(Camera& cam, float radius)
	{
		//camera->SetCameraPosition(Showcase_camPosition);
		//camera->RotateCamera(Showcase_camRotation.x, Showcase_camRotation.y, Showcase_camRotation.z);
		//for(int i=0;i<m_instanceCount*3;i+=m_instanceCount)
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
		std::cout << *(int*)val << std::endl;
		Renderer3D::InstancedFoliageData(*m_foliageMesh, m_foliageSpawnTransforms, ssbo_outTransforms);

		Renderer3D::BeginSceneFoliage(cam);
		Renderer3D::DrawFoliageInstanced(*m_foliageMesh, glm::mat4(1.0), *(int*)val, { 1,1,1,1 }, Terrain::time);
	}
	void Foliage::RenderFoliage(Camera& cam)
	{
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
		std::cout << *(int*)val << std::endl;
		Renderer3D::InstancedFoliageData(*m_foliageMesh, m_foliageSpawnTransforms, ssbo_outTransforms);

		Renderer3D::BeginSceneFoliage(cam);
		Renderer3D::DrawFoliageInstanced(*m_foliageMesh, glm::mat4(1.0), *(int*)val, { 1,1,1,1 }, Terrain::time);
	}
	glm::mat4 Foliage::getTransform(int index)
	{
		if (index >= m_foliageTransforms.size() || index < 0) {
			HAZEL_CORE_ERROR("index out of bounds of foliage transform array");
			return glm::mat4(1.0);
		}
		return m_foliageTransforms[index];
	}
	void Foliage::update(float ts)
	{
	}

	void Foliage::CalFoliageAroundCamera(Camera& cam)
	{
		//if (m_foliageSpawnTransforms.size() == 0) 
		//{
		glm::vec3 cam_pos = cam.GetCameraPosition();
		std::thread t([&]() {
			int k = 0;
			for (int i = 0; i < m_foliageTransforms.size(); i++)
			{
				auto matrix = m_foliageTransforms[i];
				glm::vec3 foliage_pos = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
				if (glm::distance(cam_pos, foliage_pos) <= 200 && k < 1000000)
				{
					m_foliageSpawnTransforms[k] = matrix;
					k++;
				}
			}
			bIsComputed = true;
			});

		t.detach();

		if (bIsComputed) {
			//Renderer3D::InstancedFoliageData(m_foliageSpawnTransforms, m_bufferID);
			bIsComputed = false;
		}
		//}
	}
	void Foliage::Vote(Camera& cam, int offset)
	{
		cs_FrustumCull->Bind();
		cs_FrustumCull->SetFloat3("camPos", cam.GetCameraPosition());
		cs_FrustumCull->SetMat4("u_ViewProjection", cam.GetProjectionView());
		cs_FrustumCull->SetInt("offset", offset);
		cs_FrustumCull->SetFloat("u_cullDistance", m_cullDistance);

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

		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
		//void* data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		//for (int i = 0; i < 512; i++)
		//	std::cout << ((int*)data)[i] << " ";
		//std::cout << std::endl;
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
	void Foliage::initialize()
	{
		cs_FoliageBuffersInit->Bind();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_voteIndices);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_voteIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_PrefixSum);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_PrefixSum);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_outTransforms);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_outTransforms);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glDispatchCompute(m_instanceCount / 1024, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	void Foliage::SpawnFoliage(glm::vec3 playerPos, float radius)
	{
		bHasSpawnned = true;
		cs_FoliageSpawn->Bind();
		cs_FoliageSpawn->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		cs_FoliageSpawn->SetInt("u_DensityMap", FOLIAGE_DENSITY_TEXTURE_SLOT);
		cs_FoliageSpawn->SetFloat3("u_PlayerPos", playerPos);
		cs_FoliageSpawn->SetFloat("u_HeightMapScale", Terrain::HeightScale);


		auto m_foliagePos = GeneratePoints(radius, { 2048,2048 }, 10); //use poisson disk distribution to populate
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

		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_foliageIndex);
		//void* data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		////for (int i = 0; i < 512; i++)
		//	std::cout << (*(int*)data) << " ";
		//std::cout << std::endl;
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
