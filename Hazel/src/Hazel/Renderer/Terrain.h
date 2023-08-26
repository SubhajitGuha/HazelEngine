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
	public:
		Terrain(float width, float height);
		~Terrain();
		void InitilizeTerrain();
		void RenderTerrain(Camera& cam);
		static float WaterLevel, HillLevel, MountainLevel ,HeightScale , FoliageHeight;
		static bool bShowTerrain, bShowWireframeTerrain;
	private:
		glm::vec2 m_dimension;
		ref<Shader> m_terrainShader,m_terrainWireframeShader;
		ref<VertexArray> m_terrainVertexArray;
		std::vector<TerrainData> terrainData;
		ref<BufferLayout> bl;
		ref<Texture2D> m_HeightMap, m_perlinNoise;
		int m_Height, m_Width, m_Channels;
		float m_maxTerrainHeight;
		std::chrono::steady_clock::time_point StartTime;
		std::vector<glm::mat4> Grass_modelMat;
	};
}