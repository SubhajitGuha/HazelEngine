#include "Hazel.h"

namespace Hazel
{
	class Foliage;
	struct TerrainData
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	class QuadTree;
	class QNode;
	class Terrain {
		friend class QuadTree;
	public:
		Terrain() {}
		Terrain(float width, float height);
		~Terrain();
		void InitilizeTerrain();
		glm::vec3 player_camera_pos;
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
	private:
		QNode* rootNode = nullptr;
		ref<QuadTree> qtree;
		glm::vec2 m_dimension;
		ref<BufferLayout> bl;
		ref<Texture2D> m_HeightMap, m_perlinNoise;
		ref<Texture2DArray> TerrainTex_Albedo, TerrainTex_Roughness, TerratinTex_Normal;
		int m_Height, m_Width, m_Channels,m_Channels1;
		float m_maxTerrainHeight;
		float max_height;
		float min_height;
		unsigned short* Height_data,*GrassSpawnArea;
		std::chrono::steady_clock::time_point StartTime;
		int CurrentChunkIndex = -1;
		float ChunkSize = 128.0f;
		uint32_t foliageBufferIndex;
	private:
		uint32_t frame_counter = 0;
		ref<Foliage> grass, Tree1,Tree2,Tree3, Bush1, Bush2, rock1, Flower, Fern;
		std::vector<ref<Foliage>> topFoliageLayer;
		std::vector<ref<Foliage>> middleFoliageLayer;
		std::vector<ref<Foliage>> bottomFoliageLayer;

		int GetChunkIndex(int PosX,int PosZ);
	};

	struct QNode //quad tree node
	{
		Bounds chunk_bounds;
		std::vector<QNode*> childrens;

		QNode() {}
		QNode(Bounds bounds)
		{
			chunk_bounds = bounds;
		}
		~QNode();

	};
	struct NodePool
	{
		static std::stack<QNode*> node_memoryPool;
		static QNode* GetNode(Bounds bounds);
		static void RecycleMemory(QNode*& node);
		static void Allocate();
		static void DeAllocate();
	};
	class QuadTree
	{
	public:
		QuadTree(Terrain* _terrain);
		void SpawnFoliageAtTile(QNode*& node, Camera& cam);
		void CreateChildren(QNode*& node, Camera& cam);
		void Insert(QNode*& node, Camera& cam);
		//this version returns all the leaf nodes
		void GetChildren(QNode*& node, std::vector<QNode*>& childrens);
		//function deletes every node at certain distance from player and also nodes which are not in view wrt player
		void DeleteNodesIfNotInScope(QNode* node, Camera& cam);
	private:
		// recursively delete nodes ,layerIDs define the startIndex and endIndex of foliage positions at 3 different layers(startID = layerIDs[n].x, endID = layerIDs[n].y)
		void DeleteNode(QNode*&);
		//check the intersection between 2 aabb
		bool aabbIntersection(Bounds& box1, Bounds& box2);
	private:
		Terrain* terrain;
	};
}