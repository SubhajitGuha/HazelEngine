#include "hzpch.h"
#include "Hazel/Log.h"
#include "Hazel/Renderer/FoliageRenderer.h"
#include "Terrain.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "Hazel//Physics/Physics3D.h"
#include "Hazel/Renderer/Antialiasing.h"
/*
	terrain class needs its own ui and modelling tools for basic prototyping.
	and also remove redundant glsl codes.
*/

namespace Hazel
{
	float Terrain::WaterLevel = 0.1, Terrain::HillLevel = 0.5, Terrain::MountainLevel = 1.0
		, Terrain::HeightScale = 200, Terrain::FoliageHeight = 6.0f;

	bool Terrain::bShowTerrain = true, Terrain::bShowWireframeTerrain = false;
	int Terrain::maxGrassAmount = 0, Terrain::ChunkIndex = 0, Terrain::RadiusOfSpawn = 1, Terrain::GrassDensity=3;
	glm::mat4 Terrain::m_terrainModelMat;
	std::vector<TerrainData> Terrain::terrainData;
	ref<VertexArray> Terrain::m_terrainVertexArray;
	float Terrain::time = 0;

	Terrain::Terrain(float width, float height)
	{
		grass = std::make_shared<Foliage>(Scene::Grass, std::ceil(1024*1024/4),512,512,300,false,50,true,true);
		Tree = std::make_shared<Foliage>(Scene::Tree, 128 * 128, 2048, 2048, 800,true,150);
		Flower = std::make_shared<Foliage>(Scene::Flower, 128 * 128*4, 512, 512, 400,false,100);
		Fern = std::make_shared<Foliage>(Scene::Fern, std::ceil(512*512/8 ), 512, 512, 100, false,50,false,true);

		maxGrassAmount = ChunkSize * ChunkSize * (pow(2*RadiusOfSpawn+1,2));//radius of spawn defines how many tiles to cover from the centre
		StartTime = std::chrono::high_resolution_clock::now();
		m_dimension.x = width;
		m_dimension.y = height;
		m_maxTerrainHeight = std::numeric_limits<float>::min();
		m_terrainShader = Shader::Create("Assets/Shaders/TerrainShader.glsl");
		m_terrainWireframeShader = Shader::Create("Assets/Shaders/TerrainWireframeShader.glsl");
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

		m_HeightMap = Texture2D::Create("Assets/Textures/Terrain_Height_Map2.png",true);
		m_perlinNoise = Texture2D::Create("Assets/Textures/PerlinTexture.png");
		TerrainTex_Albedo = Texture2D::Create("Assets/Textures/forrest_ground_01_diff_2k.jpg");
		TerrainTex_Roughness = Texture2D::Create("Assets/Textures/forrest_ground_01_rough_2k.jpg");
		TerratinTex_Normal = Texture2D::Create("Assets/Textures/forrest_ground_01_nor_gl_2k.jpg");

		//Renderer3D::AllocateInstancedFoliageData(maxGrassAmount * GrassDensity, foliageBufferIndex);

		m_terrainShader->Bind();
		m_HeightMap->Bind(HEIGHT_MAP_TEXTURE_SLOT);
		m_perlinNoise->Bind(PERLIN_NOISE_TEXTURE_SLOT);
		m_terrainShader->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
		m_terrainShader->SetInt("u_Albedo", ALBEDO_SLOT);
		m_terrainShader->SetInt("u_Roughness", ROUGHNESS_SLOT);
		m_terrainShader->SetInt("u_Normal", NORMAL_SLOT);
		m_terrainShader->SetInt("u_perlinNoise", PERLIN_NOISE_TEXTURE_SLOT);
		m_terrainShader->SetInt("diffuse_env", IRR_ENV_SLOT);
		m_terrainShader->SetInt("specular_env", ENV_SLOT);
		m_terrainShader->SetInt("SSAO", SSAO_BLUR_SLOT);
		m_terrainWireframeShader->Bind();
		m_terrainWireframeShader->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);

		m_terrainVertexArray = VertexArray::Create();

		//divide the landscape in 'n' number of patches
		float res = ChunkSize;
		//skip the edges for abrupt triangle formation
		for (int i = 2; i <= m_dimension.y-2; i+=res)
		{
			for (int j = 2; j <= m_dimension.x-2; j+=res)
			{
				TerrainData v1;
				v1.Position = glm::vec3( j, 0, i);
				v1.TexCoord = glm::vec2(j/m_dimension.x,i/m_dimension.y);
				terrainData.push_back(v1);

				TerrainData v2;
				v2.Position = glm::vec3(j + res, 0, i);
				v2.TexCoord = glm::vec2(j/m_dimension.x + res/m_dimension.x, i/m_dimension.y);				
				terrainData.push_back(v2);

				TerrainData v3;
				v3.Position = glm::vec3( j, 0, i + res);
				v3.TexCoord = glm::vec2(j/m_dimension.x, i/m_dimension.y + res/m_dimension.y);				
				terrainData.push_back(v3);

				TerrainData v4;
				v4.Position = glm::vec3(j + res, 0, i  + res);
				v4.TexCoord = glm::vec2(j/ m_dimension.x + res / m_dimension.x, i / m_dimension.y + res / m_dimension.y);
				terrainData.push_back(v4);
			}
		}

		ref<VertexBuffer> vb = VertexBuffer::Create(&terrainData[0].Position.x, sizeof(TerrainData)*terrainData.size());
		bl = std::make_shared<BufferLayout>();
		bl->push("Position", DataType::Float3);
		bl->push("coord", DataType::Float2);
		m_terrainVertexArray->AddBuffer(bl, vb);
		glPatchParameteri(GL_PATCH_VERTICES, 4);//will be present after all vertex array operations for tessellation

		glm::mat4 terrain_transform = glm::mat4(1.0f);//glm::translate(glm::mat4(1.0f), { 0,0,0 }) * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), { 0,0,1 });
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
		std::normal_distribution<float> NormalDist(0.0, 1.0);
		std::default_random_engine generator;

		CurrentChunkIndex = 0;//Let cam position at start is at 0,0
		//SpawnGrassOnChunks(0, 0, RadiusOfSpawn);

		//Terrain collision
		std::vector<int> HeightValues;
		int spacing = 32.0f;
		//in physx the data is stored in row-major format
		for (int j = 0; j < m_Width; j+=spacing) {
			for (int i = 0; i < m_Height; i+=spacing)
			{
				float y = (Height_data[i * m_Width + j + m_Channels] - min_height) / (max_height - min_height);//R channel of 1st vertex
				y *= HeightScale;

				HeightValues.push_back(ceil(y));
			}
		}
		//Physics3D::AddHeightFieldCollider(HeightValues, m_Width, m_Height, spacing, glm::mat4(1.0f));//transform is hard codded
		//stbi_image_free(Height_data);
}
	void Terrain::RenderTerrain(Camera& cam)
	{
		int CamX = cam.GetCameraPosition().x;
		int CamZ = cam.GetCameraPosition().z;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		m_terrainModelMat = glm::mat4(1.0);

		TerrainTex_Albedo->Bind(ALBEDO_SLOT);
		TerrainTex_Roughness->Bind(ROUGHNESS_SLOT);
		TerratinTex_Normal->Bind(NORMAL_SLOT);

		m_terrainShader->Bind();
		m_terrainShader->SetFloat("u_Tiling", 40);//Tiling factor for all terrain textures (not for height map)
		m_terrainShader->SetFloat("HEIGHT_SCALE", HeightScale);
		m_terrainShader->SetFloat("FoliageHeight", FoliageHeight);
		m_terrainShader->SetFloat3("DirectionalLight_Direction", Renderer3D::m_SunLightDir);
		m_terrainShader->SetFloat3("SunLight_Color", Renderer3D::m_SunColor);
		m_terrainShader->SetFloat("SunLight_Intensity", Renderer3D::m_SunIntensity);
		m_terrainShader->SetFloat("u_Intensity", Renderer3D::m_SunIntensity);
		m_terrainShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		m_terrainShader->SetMat4("u_oldProjectionView", Renderer3D::m_oldProjectionView);
		m_terrainShader->SetMat4("u_Model", m_terrainModelMat);
		m_terrainShader->SetMat4("u_View", cam.GetViewMatrix());
		m_terrainShader->SetFloat3("camPos", cam.GetCameraPosition());
		m_terrainShader->SetFloat("WaterLevel", WaterLevel);
		m_terrainShader->SetFloat("HillLevel", HillLevel);
		m_terrainShader->SetFloat("MountainLevel", MountainLevel);
		time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - StartTime).count()/1000.0;
		m_terrainShader->SetFloat("Time", time);
		//HAZEL_CORE_ERROR(time);
		if (bShowTerrain)
			RenderCommand::DrawArrays(*m_terrainVertexArray, terrainData.size(), GL_PATCHES, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		m_terrainWireframeShader->Bind();
		m_terrainWireframeShader->SetFloat("HEIGHT_SCALE", HeightScale);
		m_terrainWireframeShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		m_terrainWireframeShader->SetMat4("u_Model", m_terrainModelMat);
		m_terrainWireframeShader->SetMat4("u_View", cam.GetViewMatrix());
		m_terrainWireframeShader->SetFloat3("camPos", cam.GetCameraPosition());
		m_terrainWireframeShader->SetFloat("WaterLevel", WaterLevel);
		m_terrainWireframeShader->SetFloat("HillLevel", HillLevel);
		m_terrainWireframeShader->SetFloat("MountainLevel", MountainLevel);

		if (bShowWireframeTerrain)
			RenderCommand::DrawArrays(*m_terrainVertexArray, terrainData.size(), GL_PATCHES, 0);

		Tree->RenderFoliage(cam,30);
		grass->SetFoliageSpacing(2.0);//space between foliages
		grass->RenderFoliage(cam);
		Fern->SetFoliageSpacing(12.0f);
		Fern->RenderFoliage(cam);
		Flower->SetFoliageSpacing(20);
		Flower->RenderFoliage(cam);

	}
}