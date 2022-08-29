#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
namespace Hazel {
	class Shader {
	public:
		Shader(std::string& vertexshader, std::string& fragmentshader);
		void init();
		void UploadUniformMat4(const std::string& str,glm::mat4& UniformMat4);
		void UploadUniformInt(const std::string& str, int& UniformInt);
		void UpladUniformFloat(const std::string& str,float& UniformFloat);
	private:
		unsigned int program;
	};
}