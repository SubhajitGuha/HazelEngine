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
			RTTriangles() = default;
			RTTriangles(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2)
			{
				this->v0 = v0;
				this->v1 = v1;
				this->v2 = v2;
			}
			glm::vec3 GetCentroid() { return (v0 + v1 + v2) * 0.3333333333333f; };
			Bounds GetBounds() { return Bounds(v0, v1, v2); };
		};
		struct BVHNode
		{
			BVHNode *leftChild,*rightChild;
			uint32_t triangleStartID, triangleCount;
			glm::vec3 aabbMin, aabbMax;//bounds of the node
			BVHNode() { leftChild = nullptr, rightChild = nullptr; triangleStartID = 0, triangleCount = 0; }
		};
		struct LinearBVHNode
		{

		};

	public:
		BVH();
		void BuildBVH(BVHNode*& node, uint32_t triStartID, uint32_t triCount);
		void CreateTriangles(LoadMesh* mesh, glm::mat4& transform = glm::mat4(1.0)); //create triangles from mesh vertex positions
	private:
		float EvaluateSAH(BVHNode& node, int axis, float pos);
	private:
		BVHNode* head = nullptr;
		std::vector<BVHNode> arrBVHNode;
		std::vector<RTTriangles> arrRTTriangles;
		std::vector<int> triIndex;
		uint32_t numNodes;
	};
}