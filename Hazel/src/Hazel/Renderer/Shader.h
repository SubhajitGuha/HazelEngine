#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
namespace Hazel {
	class Shader {
	public:
		virtual ~Shader() = default;
		virtual void Bind() = 0;
		virtual void UnBind() = 0;
		virtual void UploadUniformMat4(const std::string& str,glm::mat4& UniformMat4){}
		virtual void UploadUniformInt(const std::string& str, const int& UniformInt) {}
		virtual void UpladUniformFloat(const std::string& str,float& UniformFloat){}
		virtual void UpladUniformFloat4(const std::string& str, const glm::vec4& UniformFloat4){}

		static Shader* Create(const std::string& path);
		static Shader* Create(std::string&,std::string&);
	private:
		unsigned int program;
	};
}