#include "hzpch.h"
#include "BVH.h"
#include "Hazel/Core.h"
#include "Hazel/ResourceManager.h"

namespace Hazel
{
	static int val = 0;
	BVH::BVH(LoadMesh*& mesh)
	{
		m_Mesh = mesh;
		numNodes = 0;
		uint32_t rootIndex = 0;
		//glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), { 0,0,1 }) * glm::scale(glm::mat4(1.0), glm::vec3(1.0f));
		glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), { 0,0,1 }) * glm::scale(glm::mat4(1.0), glm::vec3(0.1f));

		CreateTriangles(transform);
		UpdateMaterials();
		triIndex.resize(arrRTTriangles.size()); //making a seperate triangle index for swaping
		for (int i = 0; i < arrRTTriangles.size(); i++)
			triIndex[i] = i;
		arrLinearBVHNode.resize(arrRTTriangles.size() * 2+1);
		std::cout << "BVH Creation started " << std::endl;		
		BuildBVH(head,0, arrRTTriangles.size());
		CleanBVH(head);//identify child nodes by making their triangle count = 0
		std::cout << "BVH Creation Finished " << std::endl;
		int* offset = &numNodes;
		FlattenBVH(head,&numNodes);
		arrLinearBVHNode.resize(numNodes);
	}
	void BVH::CreateTriangles(glm::mat4& transform)
	{
		int matCount = 0;
		std::vector<std::string> texturePaths_albedo, texturePaths_roughness;
		for (auto sub_mesh : m_Mesh->m_subMeshes)
		{
			for (int i = 0; i < sub_mesh.Vertices.size(); i+=3)
			{
				auto vertices = sub_mesh.Vertices;
				auto uv = sub_mesh.TexCoord;
				glm::vec3 v0 = transform * glm::vec4(vertices[i],1.0);
				glm::vec3 v1 = transform * glm::vec4(vertices[i+1], 1.0);
				glm::vec3 v2 = transform * glm::vec4(vertices[i+2], 1.0);

				RTTriangles triangles(v0, v1, v2, uv[i],uv[i+1],uv[i+2],matCount);
				arrRTTriangles.push_back(triangles);
			}
			texturePaths_albedo.push_back(ResourceManager::allMaterials[sub_mesh.m_MaterialID]->GetAlbedoPath());
			texturePaths_roughness.push_back(ResourceManager::allMaterials[sub_mesh.m_MaterialID]->GetRoughnessPath());
			matCount++; //increment the material as we move to the next sub mesh
		}
		arrMaterials.resize(matCount);
		texArray_albedo = Texture2DArray::Create(texturePaths_albedo, matCount);
		texArray_roughness = Texture2DArray::Create(texturePaths_roughness, matCount);
	}

	void BVH::UpdateMaterials()
	{
		int k = 0;
		for (auto sub_mesh : m_Mesh->m_subMeshes) //iterate through all sub meshes and get the materials
		{
			HZ_ASSERT(!ResourceManager::allMaterials[sub_mesh.m_MaterialID]);
			auto recource_material = ResourceManager::allMaterials[sub_mesh.m_MaterialID];
			Material mat;
			mat.color = recource_material->GetColor();
			mat.emissive_col = recource_material->GetColor(); //needs change
			mat.emissive_strength = 0.0f; //needs change
			mat.metalness = recource_material->GetMetalness();
			mat.roughness = recource_material->GetRoughness();
			arrMaterials[k] = mat;
			k++;
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

	int BVH::FlattenBVH(BVHNode* node, int *offset)//dfs traversal
	{
		LinearBVHNode* linearNode = &arrLinearBVHNode[*offset];
		linearNode->aabbMax = node->aabbMax;
		linearNode->aabbMin = node->aabbMin;

		int myOffset = (*offset)++;
		if (node->triangleCount>0)
		{
			linearNode->triangleStartID = node->triangleStartID;
			linearNode->triangleCount = node->triangleCount;		
		}
		else {
			linearNode->triangleStartID = node->triangleStartID;
			linearNode->triangleCount = node->triangleCount;

			BVH::FlattenBVH(node->leftChild,offset);	
			linearNode->rightChild = BVH::FlattenBVH(node->rightChild,offset);			
		}
		return myOffset;
	}

	void BVH::CleanBVH(BVHNode* node)
	{
		if (node->leftChild == nullptr && node->rightChild == nullptr)
		{
			return;
		}
		else
		{
			node->triangleCount = 0;
			if (node->leftChild)
				CleanBVH(node->leftChild);
			if (node->rightChild)
				CleanBVH(node->rightChild);
		}
	}

	//recursively building the bvh tree and storing it as dfs format
	void BVH::BuildBVH(BVHNode*& node, int triStartID, int triCount)
	{
		node = new BVHNode();
		//++numNodes;
		Bounds bounds;
		for (int i = 0; i < triCount; i++)
		{
			auto bnds = arrRTTriangles[triIndex[i + triStartID]].GetBounds();
			bounds.Union(bnds);
		}

		node->aabbMax = bounds.aabbMax;
		node->aabbMin = bounds.aabbMin;
		node->triangleStartID = triStartID;
		node->triangleCount = triCount;

		int bestAxis = -1;
		float bestPos = 0, bestCost = 1e30f;
		for (int axis = 0; axis < 3; axis++) //evaluate for each axis
		{
			if (bounds.aabbMax[axis] == bounds.aabbMin[axis])
				continue;
			float scale = (bounds.aabbMax[axis] - bounds.aabbMin[axis]) / 100.0f; //splitting the bounds into equally spaced line intervals
			for (int i = 1; i < 100; i++)
			{
				//auto& triangle = arrRTTriangles[triIndex[triStartID + i]];
				glm::vec3 extent = bounds.aabbMax - bounds.aabbMin;
				float candidatePos = bounds.aabbMin[axis] + i*scale;//triangle.GetCentroid()[axis];
				float cost = EvaluateSAH(*node, axis, candidatePos);
				if (cost < bestCost) {
					bestPos = candidatePos;
					bestAxis = axis;
					bestCost = cost;
				}
			}
		}
		node->axis = bestAxis;//net the node axis
		if (triCount <= 2)//4 is the minimum number of triangles that a node should contain
		{
			return;
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
