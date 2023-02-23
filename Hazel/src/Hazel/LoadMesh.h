#pragma once
#include "Hazel.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace Hazel {
	class LoadMesh
	{
	public:
		LoadMesh();
		LoadMesh(const std::string& Path);
		~LoadMesh();
		void LoadObj(const std::string& Path);
	public:
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> TexCoord;
		std::vector<glm::vec3> Normal;
		std::vector<unsigned int> Vertex_Indices;
		std::vector<unsigned int> TexCoord_Indices;
	};
}
