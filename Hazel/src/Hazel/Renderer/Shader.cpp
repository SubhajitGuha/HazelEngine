#include "hzpch.h"
#include "Shader.h"
#include "RendererAPI.h"
#include "Hazel/platform/Opengl/OpenGlShader.h"
namespace Hazel {
	ref<Shader>  Shader::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlShader>(path);
		default:
			return nullptr;
		}
	}
	ref<Shader>  Shader::Create(std::string& vertexshader,std::string& fragmentshader)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None :
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlShader>(vertexshader,fragmentshader);
		default:
			return nullptr;
		}
	}
}