#include "hzpch.h"
#include "Hazel/Log.h"
#include "Terrain.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "Hazel//Physics/Physics3D.h"

namespace Hazel
{
	float Terrain::WaterLevel = 0.1, Terrain::HillLevel = 0.5, Terrain::MountainLevel = 1.0
		, Terrain::HeightScale = 600, Terrain::FoliageHeight = 6.0f;

	bool Terrain::bShowTerrain = true, Terrain::bShowWireframeTerrain = false;
	int Terrain::maxGrassAmount = 0, Terrain::ChunkIndex = 0, Terrain::RadiusOfSpawn = 1, Terrain::GrassDensity=5;
	Terrain::Terrain(float width, float height)
	{
		maxGrassAmount = ChunkSize * ChunkSize * (pow(2*RadiusOfSpawn+1,2));//radius of spawn defines how many tiles to cover from the centre
		StartTime = std::chrono::high_resolution_clock::now();
		m_dimension.x = width;
		m_dimension.y = height;
		m_maxTerrainHeight = std::numeric_limits<float>::min();
		m_terrainShader = Shader::Create("Assets/Shaders/TerrainShader.glsl");
		m_terrainWireframeShader = Shader::Create("Assets/Shaders/TerrainWireframeShader.glsl");
		Grass_modelMat.resize(maxGrassAmount * GrassDensity);
		InitilizeTerrain();
	}
	Terrain::~Terrain()
	{
		
	}
	void Terrain::InitilizeTerrain()
	{
		stbi_set_flip_vertically_on_load(1);//need to abstract
		Height_data = stbi_load_16("Assets/Textures/Terrain_Height_Map.png", &m_Width, &m_Height, &m_Channels, 0);
		//needs to have different width,height,channels
		GrassSpawnArea = stbi_load_16("Assets/Textures/grass_mask.png", &m_Width, &m_Height, &m_Channels1, 0);

		m_HeightMap = Texture2D::Create("Assets/Textures/Terrain_Height_Map.png");
		m_perlinNoise = Texture2D::Create("Assets/Textures/PerlinTexture.png");

		m_terrainShader->Bind();
		m_HeightMap->Bind(HEIGHT_MAP_TEXTURE_SLOT);
		m_perlinNoise->Bind(PERLIN_NOISE_TEXTURE_SLOT);
		m_terrainShader->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		m_terrainShader->SetInt("u_perlinNoise", PERLIN_NOISE_TEXTURE_SLOT);
		m_terrainWireframeShader->Bind();
		m_terrainWireframeShader->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);

		m_terrainVertexArray = VertexArray::Create();

		//divide the landscape in 'n' number of patches
		float res = ChunkSize;
		for (int i = 0; i < m_dimension.y; i+=res)
		{
			for (int j = 0; j < m_dimension.x; j+=res)
			{
				TerrainData v1;
				v1.Position = glm::vec3( j, 0, i);
				v1.TexCoord = glm::vec2(j/m_dimension.x,i/m_dimension.y);
				v1.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v1);

				TerrainData v2;
				v2.Position = glm::vec3(j + res, 0, i);
				v2.TexCoord = glm::vec2(j/m_dimension.x + res/m_dimension.x, i/m_dimension.y);
				v2.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v2);

				TerrainData v3;
				v3.Position = glm::vec3( j, 0, i + res);
				v3.TexCoord = glm::vec2(j/m_dimension.x, i/m_dimension.y + res/m_dimension.y);
				v3.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v3);

				TerrainData v4;
				v4.Position = glm::vec3(j + res, 0, i  + res);
				v4.TexCoord = glm::vec2(j/ m_dimension.x + res / m_dimension.x, i / m_dimension.y + res / m_dimension.y);
				v4.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v4);
			}
		}

		ref<VertexBuffer> vb = VertexBuffer::Create(&terrainData[0].Position.x, sizeof(TerrainData)*terrainData.size());
		bl = std::make_shared<BufferLayout>();
		bl->push("Position", DataType::Float3);
		bl->push("coord", DataType::Float2);
		bl->push("normal", DataType::Float3);
		m_terrainVertexArray->AddBuffer(bl, vb);
		glPatchParameteri(GL_PATCH_VERTICES, 4);//will be present after al vertex array operations

		glm::mat4 terrain_transform = glm::translate(glm::mat4(1.0f), { 0,1,0 }) * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), { 0,0,1 });
		max_height = std::numeric_limits<float>::min();
		min_height = std::numeric_limits<float>::max();

		for (int j = 0; j < m_Width; j++)
			for (int i = 0; i < m_Height; i++)
				if (max_height < Height_data[j * m_Width + i + m_Channels])
					max_height = Height_data[j * m_Width + i + m_Channels];

		for (int j = 0; j < m_Width; j++)
			for (int i = 0; i < m_Height; i++)
				if (min_height > Height_data[j * m_Width + i + m_Channels])
					min_height = Height_data[j * m_Width + i + m_Channels];

		std::uniform_real_distribution<float> RandomFloat(-1.0f, 1.0f);
		std::default_random_engine generator;

		//grass spawn
		for (int j = 0; j < m_Width; j+=1) {
			for (int i = 0; i < m_Height; i+=1)
			{					
					if (GrassSpawnArea[j * m_Width + i + m_Channels1] < 1500)
						continue;

					float y = (Height_data[j * m_Width + i + m_Channels] - min_height) / (max_height - min_height);//R channel of 1st vertex
					y *= HeightScale;

					GrassSpawnData data;
					//Store the child grass data of the current cell position
					for (int k = 0; k < GrassDensity; k++)//grass per unit
					{
						TerrainGrassData child_data;
						child_data.position = { i + RandomFloat(generator) * k / GrassDensity * 2.0,y + 2.0f, j + RandomFloat(generator) * k / GrassDensity * 2.0 };
						child_data.rotation = { RandomFloat(generator) * 20.0f,RandomFloat(generator) * 90.0f,RandomFloat(generator) * 20.0f };//in degrees
						child_data.scale = glm::vec3((RandomFloat(generator) + 1) / 2.0 + 1.0f);
						data.m_ChildGrassData.push_back(child_data);
					}
					m_GrassSpawnData[j * m_Width + i]=data;
			}
		}

		CurrentChunkIndex = 0;//Let cam position at start is at 0,0
		SpawnGrassOnChunks(0, 0, RadiusOfSpawn);

		//Terrain collision
		std::vector<int> HeightValues;
		int spacing = 16.0f;
		//in physx the data is stored in row-major format
		for (int j = 0; j < m_Width; j+=spacing) {
			for (int i = 0; i < m_Height; i+=spacing)
			{
				float y = (Height_data[i * m_Width + j + m_Channels] - min_height) / (max_height - min_height);//R channel of 1st vertex
				y *= HeightScale;

				HeightValues.push_back(ceil(y));
			}
		}
		Physics3D::AddHeightFieldCollider(HeightValues, m_Width, m_Height, spacing, glm::rotate(glm::mat4(1.0), glm::radians(180.0f), { 0,0,1 }));//transform is hard codded
		//stbi_image_free(Height_data);
}
	void Terrain::RenderTerrain(Camera& cam)
	{
		int CamX = cam.GetCameraPosition().x;
		int CamZ = cam.GetCameraPosition().z;
		

		glDisable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		glm::mat4 transform = glm::rotate(glm::mat4(1.0), glm::radians(180.0f), {0,0,1});
		m_terrainShader->Bind();
		m_terrainShader->SetFloat("HEIGHT_SCALE", HeightScale);
		m_terrainShader->SetFloat("FoliageHeight", FoliageHeight);
		m_terrainShader->SetFloat3("u_LightDir", Renderer3D::m_SunLightDir);
		m_terrainShader->SetFloat("u_Intensity", Renderer3D::m_SunIntensity);
		m_terrainShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		m_terrainShader->SetMat4("u_Model", transform);
		m_terrainShader->SetMat4("u_View", cam.GetViewMatrix());
		m_terrainShader->SetFloat3("camPos", cam.GetCameraPosition());
		m_terrainShader->SetFloat("WaterLevel", WaterLevel);
		m_terrainShader->SetFloat("HillLevel", HillLevel);
		m_terrainShader->SetFloat("MountainLevel", MountainLevel);
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - StartTime).count()/1000.0;
		m_terrainShader->SetFloat("Time", time);
		//HAZEL_CORE_ERROR(time);
		if (bShowTerrain)
			RenderCommand::DrawArrays(*m_terrainVertexArray, terrainData.size(), GL_PATCHES, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		m_terrainWireframeShader->Bind();
		m_terrainWireframeShader->SetFloat("HEIGHT_SCALE", HeightScale);
		m_terrainWireframeShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		m_terrainWireframeShader->SetMat4("u_Model", transform);
		m_terrainWireframeShader->SetMat4("u_View", cam.GetViewMatrix());
		m_terrainWireframeShader->SetFloat3("camPos", cam.GetCameraPosition());
		m_terrainWireframeShader->SetFloat("WaterLevel", WaterLevel);
		m_terrainWireframeShader->SetFloat("HillLevel", HillLevel);
		m_terrainWireframeShader->SetFloat("MountainLevel", MountainLevel);

		if (bShowWireframeTerrain)
			RenderCommand::DrawArrays(*m_terrainVertexArray, terrainData.size(), GL_PATCHES, 0);
		if (CamX < 0 && CamZ>0) 
		{
			//ChunkIndex = GetChunkIndex(CamX, CamZ);
			if (HasPlayerMovedFromChunk(CamX, CamZ))
				SpawnGrassOnChunks(CamX, CamZ, RadiusOfSpawn);

			Renderer3D::BeginSceneFoliage(cam);
			Renderer3D::DrawFoliageInstanced(*Scene::Grass, transform, Grass_modelMat.size(), { 0,0.0,0.0,1 },time, 0.4);
		}
	}

	int Terrain::GetChunkIndex(int PosX, int PosZ)
	{
		PosX = abs(PosX);
		PosZ = abs(PosZ);
		float maxChunkValX = floor(m_Width / ChunkSize);
		float maxChunkValZ = floor(m_Height / ChunkSize);
		int ChunkIndex = floor(std::min(maxChunkValX,(PosX / ChunkSize))) + floor(std::min(maxChunkValZ,(PosZ / ChunkSize))) * floor(m_Width / ChunkSize);
		return ChunkIndex;
	}

	void Terrain::SpawnGrassOnChunks(int PosX, int PosZ, int RadiusOfSpawn)
	{
		CurrentChunkIndex = GetChunkIndex(PosX, PosZ);
		std::vector<int> NeighbourChunkIndices;
		NeighbourChunkIndices.push_back(CurrentChunkIndex);//as grass will also be spawnned on the current index
		int numberOfChunksX = m_Width / floor(ChunkSize); //number of chunks horizontally (horizontal=vertical)

		//Get the neighbouring Chunks
		for (int i = -RadiusOfSpawn; i <= RadiusOfSpawn; i++)
		{
			for (int j = -RadiusOfSpawn; j <= RadiusOfSpawn; j++)
			{
				int X = CurrentChunkIndex + j;	//move Horizontally
				int Y = i * numberOfChunksX;	//move vertically
				if (X<0 || (X % numberOfChunksX) >numberOfChunksX - 1)
					continue;

				int index = X + Y;
				if (index < 0 || index == CurrentChunkIndex)
					continue;

				NeighbourChunkIndices.push_back(index);
			}
		}

		int count = 0;
		//Now get vertices for each chunk
		for (int ChunkID : NeighbourChunkIndices)//iterate through all neighbouring chunks
		{
			int minX = (ChunkID % numberOfChunksX) * floor(ChunkSize);
			int maxX = minX + floor(ChunkSize);
			int minZ = (ChunkID / numberOfChunksX) * floor(ChunkSize);
			int maxZ = minZ + floor(ChunkSize);

			//now iterate over minX to maxX and minZ to maxZ vertex positions of the chunk
			for (int i = minZ; i < maxZ; i++)
			{
				for (int j = minX; j < maxX; j++)
				{
					int coord = abs(i) * m_Width  + abs(j);//get index of the arrray that stores the grass
					if (coord > m_Width * m_Height)
						break;
					if (m_GrassSpawnData.find(coord) == m_GrassSpawnData.end())//if key is not found dont spawn the grass
						continue;

					std::vector<TerrainGrassData> grass_data = m_GrassSpawnData[coord].m_ChildGrassData;
					for (int k = 0; k < GrassDensity; k++)
					{
						glm::mat4 grass_transform = glm::translate(glm::mat4(1.0), grass_data[k].position) *
							glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), { 0,0,1 }) *
							//glm::rotate(glm::mat4(1.0), glm::radians(m_GrassData[coord].rotation.x), { 1,0,0 }) *
							glm::rotate(glm::mat4(1.0), glm::radians(grass_data[k].rotation.y), { 0,1,0 }) *
							glm::scale(glm::mat4(1.0), grass_data[k].scale);
						Grass_modelMat[count] = grass_transform;
						count++;
					}
				}
			}
		}
		//std::reverse(Grass_modelMat.begin(), Grass_modelMat.end());
		Renderer3D::InstancedFoliageData(*Scene::Grass, Grass_modelMat);
	}

	bool Terrain::HasPlayerMovedFromChunk(int PosX, int PosZ)
	{
		PosX = abs(PosX);
		PosZ = abs(PosZ);
		return CurrentChunkIndex != GetChunkIndex(PosX,PosZ);
	}
}