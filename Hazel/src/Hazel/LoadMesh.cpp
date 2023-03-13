#include "hzpch.h"
#include "LoadMesh.h"
#include "Log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Hazel {
	LoadMesh::LoadMesh()
	{
	}
	LoadMesh::LoadMesh(const std::string& Path)
		:vertices(0),Normal(0),TexCoord(0)
	{
		LoadObj(Path);
	}
	LoadMesh::~LoadMesh()
	{
	}
	void LoadMesh::LoadObj(const std::string& Path)
	{
		std::ifstream file(Path);
		if (!file) {
			HAZEL_CORE_ERROR("Mesh Cannot be loaded! File Not Found");
			return;
		}

		std::string line;
		while (std::getline(file, line))
		{
			std::string word;
			std::stringstream ss(line);
			ss >> word;
			if (word == "v")
			{
				std::string tmp;
				glm::vec3 vertex;
				ss >> tmp;
				vertex.x = std::stof(tmp);
				ss >> tmp;
				vertex.y = std::stof(tmp);
				ss >> tmp;
				vertex.z = std::stof(tmp);
				vertices.push_back(vertex);
			}
			if (word == "vt")
			{
				std::string tmp;
				glm::vec3 tex_coord;
				ss >> tmp;
				tex_coord.x = std::stof(tmp);
				ss >> tmp;
				tex_coord.y = std::stof(tmp);
				TexCoord.push_back(tex_coord);
			}
			if (word == "vn")
			{
				std::string tmp;
				glm::vec3 vertex_normals;
				ss >> tmp;
				vertex_normals.x = std::stof(tmp);
				ss >> tmp;
				vertex_normals.y = std::stof(tmp);
				ss >> tmp;
				vertex_normals.z = std::stof(tmp);
				Normal.push_back(vertex_normals);
			}
			if (word == "f")
			{
				std::string tmp;

				ss >> tmp; {
					int found_loc = 0;
					found_loc = tmp.find("/", 0);
					Vertex_Indices.push_back((unsigned int)std::stoi(tmp.substr(0, found_loc)) - 1);
					std::string res = tmp.substr(found_loc + 1, tmp.find("/", found_loc));
					//if (res.find( "/")!=-1)//check if 
						//res = "0";
					TexCoord_Indices.push_back((unsigned int)std::stoi(res) - 1);
					Normal_Indices.push_back((unsigned int)std::stoi(tmp.substr(tmp.find_last_of("/")+1)) - 1); }

				ss >> tmp; {
					int found_loc = 0;
					found_loc = tmp.find("/", 0);
					Vertex_Indices.push_back((unsigned int)std::stoi(tmp.substr(0, found_loc)) - 1);
					std::string res = tmp.substr(found_loc + 1, tmp.find("/", found_loc));
					//if (res.find("/") != -1)
						//res = "0";
					TexCoord_Indices.push_back((unsigned int)std::stoi(res) - 1);
					Normal_Indices.push_back((unsigned int)std::stoi(tmp.substr(tmp.find_last_of("/")+1)) - 1); }

				ss >> tmp; {
					int found_loc = 0;
					found_loc = tmp.find("/", 0);
					Vertex_Indices.push_back((unsigned int)std::stoi(tmp.substr(0, found_loc)) - 1);
					std::string res = tmp.substr(found_loc + 1, tmp.find("/", found_loc));

					TexCoord_Indices.push_back((unsigned int)std::stoi(res) - 1);
					std::string res1 = tmp.substr(found_loc + 1);
					Normal_Indices.push_back((unsigned int)std::stoi(tmp.substr(tmp.find_last_of("/")+1)) - 1); }

			}
		}
	}
}