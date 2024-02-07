#include "hzpch.h"
#include "BVH.h"
#include "Hazel/Core.h"
#include "glad/glad.h"
#include "Hazel/ResourceManager.h"

#define NUM_BINS 200

namespace Hazel
{
	static int val = 0;
	BVH::BVH(LoadMesh*& mesh)
	{
		m_Mesh = mesh;
		numNodes = 0;
		uint32_t rootIndex = 0;
		//glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), { 0,0,1 }) * glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
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
			ref<Texture2D> albedoTexture = Texture2D::Create(ResourceManager::allMaterials[sub_mesh.m_MaterialID]->GetAlbedoPath());
			ref<Texture2D> roughnessTexture = Texture2D::Create(ResourceManager::allMaterials[sub_mesh.m_MaterialID]->GetRoughnessPath());

			uint64_t albedo_handle = glGetTextureHandleARB(albedoTexture->GetID()); //get a handle from the gpu
			glMakeTextureHandleResidentARB(albedo_handle); //load the texture into gpu memory using the handle
			uint64_t roughness_handle = glGetTextureHandleARB(roughnessTexture->GetID());
			//if(albedo_handle!=roughness_handle)
			glMakeTextureHandleResidentARB(roughness_handle);

			for (int i = 0; i < sub_mesh.Vertices.size(); i+=3)
			{
				//auto vertices = sub_mesh.Vertices;
				//auto normals = sub_mesh.Normal;
				//auto uv = sub_mesh.TexCoord;

				//transforming the vertices and normals to world space
				glm::vec3 v0 = transform * glm::vec4(sub_mesh.Vertices[i],1.0);
				glm::vec3 v1 = transform * glm::vec4(sub_mesh.Vertices[i+1], 1.0);
				glm::vec3 v2 = transform * glm::vec4(sub_mesh.Vertices[i+2], 1.0);

				glm::vec3 n0 = transform * glm::vec4(sub_mesh.Normal[i], 0.0);
				glm::vec3 n1 = transform * glm::vec4(sub_mesh.Normal[i + 1], 0.0);
				glm::vec3 n2 = transform * glm::vec4(sub_mesh.Normal[i + 2], 0.0);

				RTTriangles triangles(v0, v1, v2, n0, n1, n2, sub_mesh.TexCoord[i], sub_mesh.TexCoord[i+1], sub_mesh.TexCoord[i+2], matCount);
				triangles.tex_albedo = albedo_handle;
				triangles.tex_roughness = roughness_handle;

				arrRTTriangles.push_back(triangles);
			}

			//texturePaths_albedo.push_back(ResourceManager::allMaterials[sub_mesh.m_MaterialID]->GetAlbedoPath());
			//texturePaths_roughness.push_back(ResourceManager::allMaterials[sub_mesh.m_MaterialID]->GetRoughnessPath());
			matCount++; //increment the material as we move to the next sub mesh
		}
		arrMaterials.resize(matCount);
		//texArray_albedo = Texture2DArray::Create(texturePaths_albedo, matCount,3);
		//texArray_roughness = Texture2DArray::Create(texturePaths_roughness, matCount,1);
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
			mat.emissive_strength = recource_material->GetEmission(); //needs change
			mat.metalness = recource_material->GetMetalness();
			mat.roughness = recource_material->GetRoughness();
			arrMaterials[k] = mat;
			k++;
		}
	}

	float BVH::EvaluateSAH(BVHNode& node, int& axis, float& pos)
	{
		float best_cost = std::numeric_limits<float>::max();
		Bounds m_Bounds; //get the total bounds of all the triangles in a particular node
		for (int i = 0; i < node.triangleCount; i++)
		{
			Bounds bnds = (arrRTTriangles[triIndex[node.triangleStartID + i]].GetBounds());
			m_Bounds.Union(bnds);
		}

		for (int a = 0; a < 3; a++) //iterate through all the axises
		{

			if (m_Bounds.aabbMax[a] == m_Bounds.aabbMin[a])
				continue;

			Bins bins[NUM_BINS]; //create bins array to store the triangle count and the bounds of the triangles in a node
			auto size = (m_Bounds.aabbMax[a] - m_Bounds.aabbMin[a]) / NUM_BINS; //get size of individual bin
			for (int i = 0; i < node.triangleCount; i++)
			{
				auto& triangle = arrRTTriangles[triIndex[node.triangleStartID + i]];
				//get the bin index
				int binIndx = std::min(NUM_BINS - 1,
					static_cast<int>(std::abs((triangle.GetCentroid()[a] - m_Bounds.aabbMin[a]) / size)));
				bins[binIndx].triangleCount++;
				bins[binIndx].bounds.Union(triangle.GetBounds());
			}

			// determine triangle counts and bounds for this split candidate
			Bounds leftBox, rightBox;
			int leftTriangleCount[NUM_BINS - 1], rightTriangleCount[NUM_BINS - 1]; //containers to store the total triangle count and area on left side of the split plane
			float leftSurfaceArea[NUM_BINS - 1], rightSurfaceArea[NUM_BINS - 1]; //containers to store the total triangle count and area on right side of the split plane
			int leftCount = 0, rightCount = 0;
			for (uint32_t i = 0; i < NUM_BINS-1; i++) //accumulate the triangle count and box area.
			{				
				leftCount += bins[i].triangleCount;
				leftBox.Union(bins[i].bounds);
				leftTriangleCount[i] = leftCount;
				leftSurfaceArea[i] = leftBox.area(); //fill from first to last

				rightCount += bins[NUM_BINS - 1 - i].triangleCount;
				rightBox.Union(bins[NUM_BINS - 1 - i].bounds);
				rightTriangleCount[NUM_BINS - 2 - i] = rightCount;
				rightSurfaceArea[NUM_BINS - 2 - i] = rightBox.area(); //fill from last to first
			}

			for (uint32_t i = 0; i < NUM_BINS - 1; i++) //evaluate cost using surface area heuristic
			{
				float cost = leftTriangleCount[i] * leftSurfaceArea[i] + rightTriangleCount[i] * rightSurfaceArea[i];
				if (cost < best_cost) 
				{
					best_cost = cost;
					axis = a;
					pos = m_Bounds.aabbMin[a] + (i+1) * size;
				}
			}
		}
		return best_cost > 0 ? best_cost : 1e30f;
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
		
		bestCost = EvaluateSAH(*node, bestAxis, bestPos);

		node->axis = bestAxis;//net the node axis
		if (triCount <= 2)//2 is the minimum number of triangles that a node should contain
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
