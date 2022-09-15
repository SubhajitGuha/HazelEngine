#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
#include "Hazel/Core.h"

namespace Hazel {
	class Shader {
	public:
		virtual ~Shader() = default;
		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void SetMat4(const std::string& str, glm::mat4& UniformMat4) = 0;
		virtual void SetInt(const std::string& str, const int& UniformInt) =0;
		virtual void SetFloat(const std::string& str,float& UniformFloat)=0;
		virtual void SetFloat4(const std::string& str, const glm::vec4& UniformFloat4)=0;

		static ref<Shader> Create(const std::string& path);
		static ref<Shader> Create(std::string&,std::string&);
	private:
		unsigned int program;
	};
}