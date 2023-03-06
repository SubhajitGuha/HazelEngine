#include "hzpch.h"
#include "CubeMapReflection.h"
#include "Hazel/platform/Opengl/OpenGlCubeMapReflection.h"

namespace Hazel {
	CubeMapReflection::CubeMapReflection()
	{
	}
	CubeMapReflection::~CubeMapReflection()
	{
	}
	ref<CubeMapReflection> CubeMapReflection::Create()
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::None:
			return nullptr;
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlCubeMapReflection>();
		}
	}
}