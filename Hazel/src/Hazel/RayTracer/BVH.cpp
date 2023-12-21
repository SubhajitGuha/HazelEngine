#include "hzpch.h"
#include "BVH.h"

namespace Hazel
{
	BVH::BVH()
	{
		numNodes = 0;
		uint32_t rootIndex = 0;
		CreateTriangles(Scene::Sphere);
		triIndex.resize(arrRTTriangles.size()); //making a seperate triangle index for swaping
		for (int i = 0; i < arrRTTriangles.size(); i++)
			triIndex[i] = i;
		arrBVHNode.resize(arrRTTriangles.size() * 2+1);
		std::cout << "BVH Creation started " << std::endl;		
		BuildBVH(head,0, arrRTTriangles.size());
		std::cout << "BVH Creation Finished " << std::endl;
		arrBVHNode.resize(numNodes);
	}
	void BVH::CreateTriangles(LoadMesh* mesh, glm::mat4& transform)
	{
		for (auto sub_mesh : mesh->m_subMeshes)
		{
			for (int i = 0; i < sub_mesh.Vertices.size(); i+=3)
			{
				auto vertices = sub_mesh.Vertices;
				RTTriangles triangles(vertices[i], vertices[i+1], vertices[i+2]);
				arrRTTriangles.push_back(triangles);
			}
		}
	}

	float BVH::EvaluateSAH(BVHNode& node, int axis, float pos)
	{
		// determine triangle counts and bounds for this split candidate
		Bounds leftBox, rightBox;
		int leftCount = 0, rightCount = 0;
		for (uint32_t i = 0; i < node.triangleCount; i++)
		{
			auto& triangle = arrRTTriangles[triIndex[node.triangleStartID + i]];
			if (triangle.GetCentroid()[axis] < pos)
			{
				leftCount++;
				leftBox.Union(triangle.GetBounds());				
			}
			else
			{
				rightCount++;
				rightBox.Union(triangle.GetBounds());				
			}
		}
		float cost = leftCount * leftBox.area() + rightCount * rightBox.area();
		return cost > 0 ? cost : 1e30f;
	}

	//recursively building the bvh tree and storing it as dfs format
	void BVH::BuildBVH(BVHNode*& node, uint32_t triStartID, uint32_t triCount)
	{
		node = new BVHNode();
		++numNodes;
		Bounds bounds;
		for (int i = 0; i < triCount; i++)
		{
			auto bnds = arrRTTriangles[triIndex[i + triStartID]].GetBounds();
			bounds.Union(bnds);
		}
 		//std::cout << bounds.aabbMax.x << " " << bounds.aabbMax.y << " " << bounds.aabbMax.z<<"\n";
		//std::cout << bounds.aabbMin.x << " " << bounds.aabbMin.y << " " << bounds.aabbMin.z << "\n";

		node->aabbMax = bounds.aabbMax;
		node->aabbMin = bounds.aabbMin;
		node->triangleStartID = triStartID;
		node->triangleCount = triCount;

		if (triCount <= 4)//4 is the minimum number of triangles that a node should contain
		{
			return;
		}
		int bestAxis = -1;
		float bestPos = 0, bestCost = 1e30f;
		for (int axis = 0; axis < 3; axis++) //evaluate for each axis
		{
			for (uint32_t i = 0; i < triCount; i++)
			{
				auto& triangle = arrRTTriangles[triIndex[triStartID + i]];
				float candidatePos = triangle.GetCentroid()[axis];
				float cost = EvaluateSAH(*node, axis, candidatePos);
				if (cost < bestCost) {
					bestPos = candidatePos;
					bestAxis = axis;
					bestCost = cost;
				}
			}
		}
		int axis = bestAxis;
		glm::vec3 extent = bounds.aabbMax - bounds.aabbMin;
		float splitPos = bestPos;

		float parentArea = extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
		float parentCost = triCount * parentArea;
		if (bestCost >= parentCost) 
			return;

		int i = triStartID, j = i + triCount - 1;
		//do partial sorting
		while (i <= j)
		{
			if (arrRTTriangles[triIndex[i]].GetCentroid()[axis] < splitPos)
				i++;
			else
				std::swap(triIndex[i], triIndex[j--]);
		}

		int leftCount = i - triStartID;
		if (leftCount == 0 || leftCount == triCount)
		{		
			return;
		}
		BVH::BuildBVH(node->leftChild, triStartID, leftCount);
		BVH::BuildBVH(node->rightChild, i, triCount - leftCount);
	}
}
