#include "hzpch.h"
#include "Shader.h"
#include "RendererAPI.h"
#include "Hazel/platform/Opengl/OpenGlShader.h"
namespace Hazel {
	Shader* Shader::Create(std::string& vertexshader,std::string& fragmentshader)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None :
			return nullptr;
		case GraphicsAPI::OpenGL:
			return new OpenGlShader(vertexshader,fragmentshader);
		default:
			return nullptr;
		}
	}
}