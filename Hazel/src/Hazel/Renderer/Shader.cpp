#include "hzpch.h"
#include "Shader.h"
#include "glad/glad.h"

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

	unsigned int program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glUseProgram(program);
}

void Hazel::Shader::init()
{
}
