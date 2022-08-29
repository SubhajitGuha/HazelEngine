#include "hzpch.h"
#include "Shader.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"

Hazel::Shader::Shader(std::string& vertexshader, std::string& fragmentshader)
{
	
		unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
		const char* chr = vertexshader.c_str();
		glShaderSource(vs, 1, &chr, nullptr);
		glCompileShader(vs);
		{
			int id;
		glGetShaderiv(vs, GL_COMPILE_STATUS, &id);
		if (id == GL_FALSE)//if the shader code is not successfully compiled
		{
			int length;
			glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &length);
			char* message = new char[length];
			glGetShaderInfoLog(vs, length, &length, message);
			HAZEL_CORE_ERROR(message);
		}

		}

		unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
		const char* chr1 = fragmentshader.c_str();
		glShaderSource(fs, 1, &chr1, nullptr);
		glCompileShader(fs);
		int id;
		glGetShaderiv(fs, GL_COMPILE_STATUS, &id);
		if (id) 
		{
			int length;
			glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &length);
			char* message = new char[length];
			glGetShaderInfoLog(fs, length, &length, message);
			HAZEL_CORE_ERROR(message);
		}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glUseProgram(program);
}

void Hazel::Shader::init()
{
}

void Hazel::Shader::UploadUniformMat4(const std::string& str,glm::mat4& UniformMat4)
{
	unsigned int location = glGetUniformLocation(program, str.c_str());
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(UniformMat4));

}

void Hazel::Shader::UploadUniformInt(const std::string& str, int& UniformInt)
{
}

void Hazel::Shader::UpladUniformFloat(const std::string& str,float& UniformFloat)
{
}
