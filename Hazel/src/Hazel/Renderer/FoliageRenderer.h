#include "Hazel.h"

namespace Hazel
{
	struct FrustumPlanes
	{
		glm::vec4 front, back, left, right; //(normal.x,normal.y,normal.z,distance);
	};
	class Foliage
	{
	public:
		//Foliage(LoadMesh* mesh);
		Foliage::Foliage(LoadMesh* mesh, uint32_t numInstances, uint32_t , uint32_t);
		~Foliage();
		void addInstance(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		void RenderFoliage(Camera& cam, float );
		glm::mat4 getTransform(int index);
		void update(float ts);
		//void addWindParam();
		void CalFoliageAroundCamera(Camera& cam);
		static glm::vec3 Showcase_camPosition;
		static glm::vec3 Showcase_camRotation;

	private:
		//frustum cull is divided in 3 stages
		void Vote(Camera&,int offset);
		void Scan(int offset);
		void Compact(int offset);
		void initialize();
		void SpawnFoliage(glm::vec3 playerPos, float);
		std::vector<glm::vec2> GeneratePoints(float radius, glm::vec2 sampleRegionSize, int numSamplesBeforeRejection = 30);
	private:
		Camera* camera;
		bool bIsComputed = false;
		int totalSum = 0, instanceCount = 0; //accmulated result of prefix sum for each 1024 dispatch units
		uint32_t ssbo_inTransforms = -1, ssbo_outTransforms = -1, ssbo_voteIndices = -1, ssbo_PrefixSum=-1, ssbo_totalPrefixSum = -1;//shader storage buffer		
		uint32_t ssbo_foliagePos = -1;
		ref<Texture2D> FoliageDensityMap;
		ref<Shader> cs_FrustumCull, cs_PrefixSum, cs_FrustumCullCompact, cs_FoliageSpawn, cs_FoliageBuffersInit;
		glm::vec2 oldPlayerPos = {256,256}; //needs refactoring
		//std::vector<glm::vec2> m_foliagePos;
		void* gpuData;
		LoadMesh* m_foliageMesh;
		uint32_t m_instanceCount;
		glm::ivec2 m_coverage;
		std::vector<glm::mat4> m_foliageTransforms,m_foliageSpawnTransforms;
		uint32_t m_bufferID = -1;
	};
}