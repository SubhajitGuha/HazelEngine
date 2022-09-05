#include "hzpch.h"
#include "OpenGlShader.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"

Hazel::OpenGlShader::OpenGlShader(std::string& vertexshader, std::string& fragmentshader)
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

Hazel::OpenGlShader::~OpenGlShader()
{
	glDeleteProgram(program);
}

void Hazel::OpenGlShader::Bind()
{
	glUseProgram(program);
}

void Hazel::OpenGlShader::UnBind()
{
	glUseProgram(0);
}

void Hazel::OpenGlShader::UploadUniformMat4(const std::string& str, glm::mat4& UniformMat4)
{
	unsigned int location = glGetUniformLocation(program, str.c_str());
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(UniformMat4));

}

void Hazel::OpenGlShader::UploadUniformInt(const std::string& str, int& UniformInt)
{
}

void Hazel::OpenGlShader::UpladUniformFloat(const std::string& str, float& UniformFloat)
{
}

void Hazel::OpenGlShader::UpladUniformFloat4(const std::string& str, const glm::vec4& UniformFloat4)
{
	unsigned int location = glGetUniformLocation(program, &str[0]);
	glUniform4f(location, UniformFloat4.r, UniformFloat4.g, UniformFloat4.b, UniformFloat4.a);
}
