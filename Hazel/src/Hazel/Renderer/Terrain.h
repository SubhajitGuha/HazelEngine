#include "Hazel.h"

namespace Hazel
{
	struct TerrainData
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec3 Normal;
	};
	class Terrain {
		struct TerrainGrassData
		{
			glm::vec3 position;
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
		};
		struct GrassSpawnData
		{
			std::vector<TerrainGrassData> m_ChildGrassData;
		};
	public:
		Terrain(float width, float height);
		~Terrain();
		void InitilizeTerrain();
		void RenderTerrain(Camera& cam);
		static float WaterLevel, HillLevel, MountainLevel ,HeightScale , FoliageHeight;
		static bool bShowTerrain, bShowWireframeTerrain;
		static int maxGrassAmount, ChunkIndex, RadiusOfSpawn, GrassDensity;
	private:
		glm::vec2 m_dimension;
		ref<Shader> m_terrainShader,m_terrainWireframeShader;
		ref<VertexArray> m_terrainVertexArray;
		std::vector<TerrainData> terrainData;
		ref<BufferLayout> bl;
		ref<Texture2D> m_HeightMap, m_perlinNoise, TerrainTex_Albedo, TerrainTex_Roughness, TerratinTex_Normal;
		int m_Height, m_Width, m_Channels,m_Channels1;
		float m_maxTerrainHeight;
		float max_height;
		float min_height;
		unsigned short* Height_data,*GrassSpawnArea;
		std::chrono::steady_clock::time_point StartTime;
		std::unordered_map<int,GrassSpawnData> m_GrassSpawnData;
		std::vector<glm::mat4> Grass_modelMat;
		int CurrentChunkIndex = -1;
		float ChunkSize = 128.0f;
	private:
		int GetChunkIndex(int PosX,int PosZ);
		void SpawnGrassOnChunks(int PosX, int PosZ, int RadiusOfSpawn = 1);
		bool HasPlayerMovedFromChunk(int PosX, int PosZ);
	};
}