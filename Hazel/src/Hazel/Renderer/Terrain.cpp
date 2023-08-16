#include "hzpch.h"
#include "Hazel/Log.h"
#include "Terrain.h"
#include "glad/glad.h"
#include "stb_image.h"
namespace Hazel
{
	float Terrain::WaterLevel = 0.1, Terrain::HillLevel = 0.5, Terrain::MountainLevel = 1.0
		, Terrain::HeightScale = 1000, Terrain::FoliageHeight = 6.0f;

	bool Terrain::bShowTerrain = true, Terrain::bShowWireframeTerrain = false;

	Terrain::Terrain(float width, float height)
	{
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
		m_HeightMap = Texture2D::Create("Assets/Textures/Terrain_Height_Map2.png");
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
		float res = 32.0f;
		for (int i = 0; i < m_dimension.y / res; i++)
		{
			for (int j = 0; j < m_dimension.y / res; j++)
			{
				TerrainData v1;
				v1.Position = glm::vec3(-m_dimension.x / 2 + j*res, 0, -m_dimension.y / 2 + i*res);
				v1.TexCoord = glm::vec2(j*res/m_dimension.x,i*res/m_dimension.y);
				v1.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v1);

				TerrainData v2;
				v2.Position = glm::vec3(-m_dimension.x / 2 + j*res + res, 0, -m_dimension.y / 2 + i*res);
				v2.TexCoord = glm::vec2(j*res/m_dimension.x + res/m_dimension.x, i*res/m_dimension.y);
				v2.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v2);

				TerrainData v3;
				v3.Position = glm::vec3(-m_dimension.x / 2 + j*res, 0, -m_dimension.y / 2 + i*res + res);
				v3.TexCoord = glm::vec2(j*res/m_dimension.x, i*res/m_dimension.y + res/m_dimension.y);
				v3.Normal = glm::vec3(0, 0, 0);
				terrainData.push_back(v3);

				TerrainData v4;
				v4.Position = glm::vec3(-m_dimension.x / 2 + j * res + res, 0, -m_dimension.y / 2 + i * res + res);
				v4.TexCoord = glm::vec2(j * res / m_dimension.x + res / m_dimension.x, i * res / m_dimension.y + res / m_dimension.y);
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
	}
	void Terrain::RenderTerrain(Camera& cam)
	{
		glDisable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { 0,1,0 }) * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), {0,0,1});
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

	}
}