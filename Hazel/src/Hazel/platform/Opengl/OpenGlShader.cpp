#include "hzpch.h"
#include "OpenGlShader.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"

namespace Hazel {

	OpenGlShader::OpenGlShader(std::string& vertexshader, std::string& fragmentshader)
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

	OpenGlShader::OpenGlShader(const std::string& path)
	{
		m_shaders = ParseFile(path);
		unsigned int vs = CompileShader(m_shaders.VertexShader,GL_VERTEX_SHADER);
		unsigned int fs = CompileShader(m_shaders.Fragmentshader,GL_FRAGMENT_SHADER);

		program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);
		glValidateProgram(program);

		glUseProgram(program);
	}

	OpenGlShader::~OpenGlShader()
	{
		glDeleteProgram(program);
	}

	unsigned int OpenGlShader::CompileShader(std::string& Shader,unsigned int type)
	{
		unsigned int s = glCreateShader(type);
		const char* chr = Shader.c_str();
		glShaderSource(s, 1, &chr, nullptr);
		glCompileShader(s);

		int id;
		glGetShaderiv(s, GL_COMPILE_STATUS, &id);
		if (id == GL_FALSE)//if the shader code is not successfully compiled
		{
			int length;
			glGetShaderiv(s, GL_INFO_LOG_LENGTH, &length);
			char* message = new char[length];
			glGetShaderInfoLog(s, length, &length, message);
			HAZEL_CORE_ERROR(message);
		}

		return s;
	}

	Shaders OpenGlShader::ParseFile(const std::string& path)
	{
		enum type {
			VERTEX_SHADER=0, FRAGMENT_SHADER=1
		};

		std::ifstream stream(path);
		if (&stream == nullptr)
			HAZEL_CORE_ERROR("Shader File Not Found!!");
		std::string ShaderCode;
		std::string Shader[2];
		int index;
		while (std::getline(stream, ShaderCode))
		{
			if (ShaderCode.find("#shader vertex") != std::string::npos)
			{
				index = VERTEX_SHADER;
				continue;
			}
			if (ShaderCode.find("#shader fragment") != std::string::npos)
			{
				index = FRAGMENT_SHADER;
				continue;
			}

			Shader[index].append(ShaderCode + "\n");
		}
		return { Shader[0],Shader[1] };
	}

	void OpenGlShader::Bind()
	{
		glUseProgram(program);
	}

	void OpenGlShader::UnBind()
	{
		glUseProgram(0);
	}

	void OpenGlShader::SetMat4(const std::string& str, glm::mat4& UniformMat4, size_t count)
	{
		UploadUniformMat4(str, UniformMat4,count);
	}

	void OpenGlShader::SetInt(const std::string& str, const int& UniformInt)
	{
		UploadUniformInt(str, UniformInt);
	}

	void OpenGlShader::SetFloat(const std::string& str,const float& UniformFloat)
	{
		UpladUniformFloat(str, UniformFloat);
	}

	void OpenGlShader::SetFloatArray(const std::string& str, float& UniformFloatArr,size_t count)
	{
		UpladUniformFloatArray(str, count, UniformFloatArr);
	}

	void OpenGlShader::SetFloat4(const std::string& str, const glm::vec4& UniformFloat4)
	{
		UpladUniformFloat4(str, UniformFloat4);
	}

	void OpenGlShader::SetFloat3(const std::string& str, const glm::vec3& UniformFloat3)
	{
		UpladUniformFloat3(str, UniformFloat3);
	}

	void OpenGlShader::SetFloat3Array(const std::string& str,const float* pointer, size_t count)
	{
		UpladUniformFloat3Array(str, pointer, count);
	}

	void OpenGlShader::SetIntArray(const std::string& str, const size_t size, const void* pointer)
	{
		UploadIntArray(str, size, pointer);
	}

	//opengl specific upload uniform
	void OpenGlShader::UploadUniformMat4(const std::string& str, glm::mat4& UniformMat4, size_t count)
	{
		unsigned int location = glGetUniformLocation(program, str.c_str());
		glUniformMatrix4fv(location, count, false, glm::value_ptr(UniformMat4));

	}

	void OpenGlShader::UploadUniformInt(const std::string& str, const int& UniformInt)
	{
		unsigned int location = glGetUniformLocation(program, str.c_str());
		glUniform1i(location, UniformInt);
	}

	void OpenGlShader::UploadIntArray(const std::string& str, const size_t size, const void* pointer)
	{
		auto location = glGetUniformLocation(program, str.c_str());
		glUniform1iv(location, size,(const GLint*) pointer);
	}

	void OpenGlShader::UpladUniformFloat(const std::string& str,const float& UniformFloat)
	{
		unsigned int location = glGetUniformLocation(program, str.c_str());
		glUniform1f(location, UniformFloat);
	}

	void OpenGlShader::UpladUniformFloatArray(const std::string& str,size_t count, float& UniformFloatArr)
	{
		unsigned int location = glGetUniformLocation(program, str.c_str());
		glUniform1fv(location,count, &UniformFloatArr);
	}

	void OpenGlShader::UpladUniformFloat4(const std::string& str, const glm::vec4& UniformFloat4)
	{
		unsigned int location = glGetUniformLocation(program, &str[0]);
		glUniform4f(location, UniformFloat4.r, UniformFloat4.g, UniformFloat4.b, UniformFloat4.a);
	}
	void OpenGlShader::UpladUniformFloat3(const std::string& str, const glm::vec3& UniformFloat3)
	{
		uint32_t location = glGetUniformLocation(program, str.c_str());
		glUniform3f(location, UniformFloat3.x, UniformFloat3.y, UniformFloat3.z);
	}
	void OpenGlShader::UpladUniformFloat3Array(const std::string& str,const float *pointer , size_t count)
	{
		uint32_t location = glGetUniformLocation(program, str.c_str());
		glUniform3fv(location, count,(const GLfloat*) pointer);
	}
}