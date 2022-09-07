#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
#include "Hazel//Renderer/Shader.h"
namespace Hazel {
	class OpenGlShader : public Shader {
	public:
		OpenGlShader(std::string& vertexshader, std::string& fragmentshader);
		~OpenGlShader();
		void Bind()override;
		void UnBind()override;
		void UploadUniformMat4(const std::string& str, glm::mat4& UniformMat4)override;
		void UploadUniformInt(const std::string& str,const int& UniformInt)override;
		void UpladUniformFloat(const std::string& str, float& UniformFloat)override;
		void UpladUniformFloat4(const std::string& str, const glm::vec4& UniformFloat4)override;

	private:
		unsigned int program;
	};
}