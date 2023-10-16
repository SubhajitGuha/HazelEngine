#include "Hazel.h"

namespace Hazel
{
	class Foliage;
	struct TerrainData
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};
	class Terrain {
		enum FOLIAGE_TYPE
		{
			GRASS, FLOWER, TREE, NONE
		};
		struct TerrainFoliageData
		{
			glm::vec3 position;
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
			FOLIAGE_TYPE f_type;
		};
		struct GrassSpawnData
		{
			std::vector<TerrainFoliageData> m_ChildGrassData;
		};
	public:
		Terrain(float width, float height);
		~Terrain();
		void InitilizeTerrain();
		static float WaterLevel, HillLevel, MountainLevel ,HeightScale , FoliageHeight;
		static bool bShowTerrain, bShowWireframeTerrain;
		static int maxGrassAmount, ChunkIndex, RadiusOfSpawn, GrassDensity;
		static glm::mat4 m_terrainModelMat;
		static std::vector<TerrainData> terrainData;
		static ref<VertexArray> m_terrainVertexArray;
		ref<Shader> m_terrainShader,m_terrainWireframeShader;
		static float time;
	public:
		void RenderTerrain(Camera& cam);
		void RenderTerrainGrass();
	private:
		glm::vec2 m_dimension;
		ref<Shader> foliageShader_instanced;
		ref<BufferLayout> bl;
		ref<Texture2D> m_HeightMap, m_perlinNoise, TerrainTex_Albedo, TerrainTex_Roughness, TerratinTex_Normal;
		int m_Height, m_Width, m_Channels,m_Channels1;
		float m_maxTerrainHeight;
		float max_height;
		float min_height;
		unsigned short* Height_data,*GrassSpawnArea;
		std::chrono::steady_clock::time_point StartTime;
		std::unordered_map<int,GrassSpawnData> m_GrassSpawnData;
		std::vector<glm::mat4> Grass_modelMat, Flower_modelMat;
		int CurrentChunkIndex = -1;
		float ChunkSize = 128.0f;
		uint32_t foliageBufferIndex;
	private:
		ref<Foliage> grass;
		int GetChunkIndex(int PosX,int PosZ);
		void SpawnGrassOnChunks(int PosX, int PosZ, int RadiusOfSpawn = 1);
		bool HasPlayerMovedFromChunk(int PosX, int PosZ);
		void FillGrassData(const TerrainFoliageData& grass_data, int index);
		void FillFlowerData(const TerrainFoliageData& flower_data, int index);

	};
}