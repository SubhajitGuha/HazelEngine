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
		struct DrawArraysIndirectCommand {
			uint32_t  count = 0;
			uint32_t  instanceCount = 0;
			uint32_t  first = 0;
			uint32_t  baseInstance = 0;
		};
		/*mesh : Foliage Mesh
		numInstances : total number of Instances that will be used
		coverageX,coverageY : How much area(sq m) will the foliage cover (must be a power of 2) eg 512,512
		cullDistance : defines the maximum distance at which the foliage will be rendered
		castShadow : Will this foliage object cast shadow (turning it off will give performance)
		LOD_Distance : max distance after which lower LOD will replace the higher ones (as this engine has 2 lods)
		applyGradientMask : applies a grigent (dark color at bottom bright color at top)
		*/
		Foliage::Foliage(LoadMesh* mesh, uint32_t numInstances, uint32_t coverageX, uint32_t coverageY, 
			float cullDistance=100, bool canCastShadow = false, float LOD_Distance = 50.0f, bool applyGradientMask = false, bool enableWind = false);
		~Foliage();
		//For foliage placement in cpu side (not in use)
		void addInstance(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		//This version distributes foliage across the coverage area using poisson distribution
		void RenderFoliage(Camera& cam, float radius);
		//this version spawns foliage in everry area(per pixel)
		void RenderFoliage(Camera& cam);
		//this version is used by the shadows renderer to cast shadows
		void RenderFoliage(ref<Shader>& shadow_shader);
		//What will be the space between each foliage object
		inline void SetFoliageSpacing(float spacing) { m_Spacing = spacing; }
		glm::mat4 getTransform(int index);
		//Get BufferID of the transform matrix after the culling
		inline uint32_t GetBufferID() { return ssbo_outTransforms; }
		inline uint32_t GetBufferID_LOD0() { return ssbo_outTransformsLOD0; }
		inline uint32_t GetBufferID_LOD1() { return ssbo_outTransformsLOD1; }
		inline LoadMesh* GetMesh() { return m_foliageMesh; }
	public:
		static glm::vec3 Showcase_camPosition;
		static glm::vec3 Showcase_camRotation;
		static std::vector<Foliage*> foliageObjects;
		//for lods
		uint32_t renderedInstanceCount,NumLOD0 = 0, NumLOD1 = 0;
		bool bCanCastShadow = false;
	private:
		//frustum cull is divided in 3 stages

		void Vote(Camera&,int offset);
		void Scan(int offset);
		void Compact(int offset);
		void CreateLODs(Camera& cam);
		void SpawnFoliage(glm::vec3 playerPos, float);
		void SpawnFoliage(glm::vec3 playerPos);
		std::vector<glm::vec2> GeneratePoints(float radius, glm::vec2 sampleRegionSize, int numSamplesBeforeRejection = 30);
	private:
		Camera* camera;
		bool bHasSpawnned = false;
		bool applyGradientMask = false;
		bool enableWind = false;
		float m_Spacing = 1.0;
		float m_cullDistance;
		bool bIsComputed = false;
		int totalSum = 0, instanceCount = 0; //accmulated result of prefix sum for each 1024 dispatch units
		uint32_t ssbo_inTransforms = -1, ssbo_outTransforms = -1, ssbo_voteIndices = -1, ssbo_PrefixSum=-1, ssbo_totalPrefixSum = -1;//shader storage buffer		
		uint32_t ssbo_foliagePos = -1;
		
		//for LODs........................................................................
		float lod0Distance = 50.0f; //for now engine supports only 2 LODs (high(0) and Low(1))
		uint32_t x = 0; //variable to initilize the atomic counters
		ref<Shader> cs_createLod; //compute shader that prepares 2 transform matrices(LOD0,LOD1) from the culled matrix
		uint32_t atomicCounter_lod0 = -1, atomicCounter_lod1 = -1; //Atomic_counter_Buffer IDs
		uint32_t ssbo_outTransformsLOD0 = -1, ssbo_outTransformsLOD1 = -1; //2 transform matrix buffer IDs
		uint32_t ssbo_indirectBuffer_LOD0 = -1, ssbo_indirectBuffer_LOD1 = -1;
		DrawArraysIndirectCommand indirectBuffer_LOD0, indirectBuffer_LOD1; //

		uint32_t m_instanceCount;
		ref<Texture2D> FoliageDensityMap; //custom density map
		ref<Shader> cs_FrustumCullVote, cs_PrefixSum, cs_FrustumCullCompact, cs_FoliageSpawn,
			cs_GrassPlacement, cs_FoliageBuffersInit, cs_CopyIndirectBufferData;
		glm::vec2 oldPlayerPos = {20000,20000}; //needs refactoring
		//void* gpuData;
		LoadMesh* m_foliageMesh; //Foliage Mesh That is used
		glm::ivec2 m_coverage;
		std::vector<glm::mat4> m_foliageTransforms,m_foliageSpawnTransforms;
	};
}