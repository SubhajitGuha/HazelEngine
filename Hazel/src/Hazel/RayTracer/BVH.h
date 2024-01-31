#pragma once
#include "Hazel.h"
namespace Hazel
{
	class BVH
	{
	public:
		struct Bounds
		{
			glm::vec3 aabbMax, aabbMin;
			Bounds() {
				float minNum = std::numeric_limits<float>::lowest();
				float maxNum = std::numeric_limits<float>::max();
				aabbMin = glm::vec3(maxNum, maxNum, maxNum);
				aabbMax = glm::vec3(minNum, minNum, minNum);
			}
			Bounds(glm::vec3& p)
			{
				aabbMax = glm::max(aabbMax, p);
				aabbMin = glm::min(aabbMin, p);
			}
			Bounds(glm::vec3& a, glm::vec3& b, glm::vec3& c)
			{
				aabbMax = glm::max(glm::max(a, b), c);
				aabbMin = glm::min(glm::min(a, b), c);
			}
			void Union(const Bounds& bounds) 
			{
				aabbMax = glm::max(aabbMax, bounds.aabbMax);
				aabbMin = glm::min(aabbMin, bounds.aabbMin);
			}
			float area() //get surface area of the box
			{
				glm::vec3 e = aabbMax - aabbMin; // box extent
				return e.x * e.y + e.y * e.z + e.z * e.x;
			}
		};
		struct RTTriangles
		{
			glm::vec3 v0, v1, v2;
			glm::vec3 n0, n1, n2;
			glm::vec2 uv0,uv1,uv2;
			uint64_t tex_albedo; //handle value for bindless albedo texture
			uint64_t tex_roughness; //handle value for bindless roughness texture
			int materialID;

			//int materialID;
			RTTriangles() = default;
			RTTriangles(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& n0, glm::vec3& n1, glm::vec3& n2,
				glm::vec2& uv0, glm::vec2& uv1, glm::vec2& uv2, int materialID)
			{
				this->v0 = v0;
				this->v1 = v1;
				this->v2 = v2;
				this->n0 = n0;
				this->n1 = n1;
				this->n2 = n2;
				this->uv0 = uv0;
				this->uv1 = uv1;
				this->uv2 = uv2;
				this->materialID = materialID;
			}
			glm::vec3 GetCentroid() { return (v0 + v1 + v2) * 0.3333333333333f; };
			Bounds GetBounds() { return Bounds(v0, v1, v2); };
		};
		struct BVHNode
		{
			BVHNode *leftChild,*rightChild;
			int axis;
			int triangleStartID, triangleCount;
			glm::vec3 aabbMin, aabbMax;//bounds of the node
			BVHNode() { leftChild = nullptr, rightChild = nullptr; triangleStartID = 0, triangleCount = 0; }
		};
		struct LinearBVHNode
		{
			int rightChild;
			int triangleStartID, triangleCount;
			glm::vec3 aabbMin, aabbMax;
		};
		struct Bins
		{
			Bounds bounds;
			int triangleCount=0;
		};
		enum SplitMethod
		{SAH,MEAN};
		struct Material
		{
			glm::vec4 color;
			float roughness;
			float metalness;
			glm::vec4 emissive_col;
			float emissive_strength;
		};
	public:
		BVH()=default;
		BVH(LoadMesh*& mesh);
		void UpdateMaterials();
	private:
		void BuildBVH(BVHNode*& node, int triStartID, int triCount);
		void CreateTriangles(glm::mat4& transform = glm::mat4(1.0)); //create triangles from mesh vertex positions
		float EvaluateSAH(BVHNode& node, int& axis, float& pos); //returns cost after finding best axis and position
		int FlattenBVH(BVHNode* node,int* offset);
		void CleanBVH(BVHNode* node);//removes count of triangles from child nodes
	private:
		LoadMesh* m_Mesh;
		BVHNode* head = nullptr;		
	public:
		std::vector<Material> arrMaterials;
		std::vector<ref<Texture2D>> tex_albedo;
		std::vector<ref<Texture2D>> tex_roughness;

		std::vector<LinearBVHNode> arrLinearBVHNode;//to be sent on to the gpu
		std::vector<RTTriangles> arrRTTriangles;//to be sent on to the gpu
		std::vector<int> triIndex;//to be sent on to the gpu
		int numNodes;
	};
}