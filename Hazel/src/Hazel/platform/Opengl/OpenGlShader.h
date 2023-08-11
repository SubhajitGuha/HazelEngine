#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
#include "Hazel//Renderer/Shader.h"
namespace Hazel {

	struct Shaders {
		std::string VertexShader;
		std::string Fragmentshader;
		std::string GeometryShader;
		std::string TessellationControlShader;
		std::string TessellationExecutionShader;
		//compute shader, tess shader....
	};

	class OpenGlShader : public Shader {
	public:
		OpenGlShader(std::string& vertexshader, std::string& fragmentshader);
		OpenGlShader(const std::string& path);
		~OpenGlShader();

		unsigned int CompileShader(std::string&,unsigned int);
		Shaders ParseFile(const std::string& path);

		void Bind()override;
		void UnBind()override;

		void SetMat4(const std::string& str, glm::mat4& UniformMat4, size_t count=1) override;
		void SetInt(const std::string& str, const int& UniformInt) override;
		void SetIntArray(const std::string& str, const size_t size, const void* pointer) override;
		void SetFloat(const std::string& str,const float& UniformFloat) override;
		virtual void SetFloatArray(const std::string& str, float& UniformFloat, size_t count) override;
		void SetFloat4(const std::string& str, const glm::vec4& UniformFloat4) override;
		void SetFloat3(const std::string& str, const glm::vec3& UniformFloat4) override;
		virtual void SetFloat3Array(const std::string& str,const float* pointer , size_t count) override;

	private://opengl specific
		void UploadUniformMat4(const std::string& str, glm::mat4& UniformMat4, size_t count = 1);
		void UploadUniformInt(const std::string& str,const int& UniformInt);
		void UploadIntArray(const std::string& str, const size_t size, const void* pointer);
		void UpladUniformFloat(const std::string& str,const float& UniformFloat);
		void UpladUniformFloatArray(const std::string& str, size_t count, float& UniformFloat);
		void UpladUniformFloat4(const std::string& str, const glm::vec4& UniformFloat4);
		void UpladUniformFloat3(const std::string& str, const glm::vec3& UniformFloat3);
		void UpladUniformFloat3Array(const std::string& str, const float* pointer, size_t count);
		unsigned int program;
		Shaders m_shaders;
	};
}