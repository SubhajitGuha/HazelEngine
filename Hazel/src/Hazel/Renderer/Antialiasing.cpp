#include "hzpch.h"
#include "Antialiasing.h"
#include "Hazel/platform/Opengl/OpenGlAntialiasing.h"

namespace Hazel
{
	int Antialiasing::frameCount = 0;
	Antialiasing::Antialiasing()
	{
	}
	Antialiasing::~Antialiasing()
	{
	}
	ref<Antialiasing> Antialiasing::Create(int width, int height)
	{
		switch (RendererAPI::GetAPI())
		{
		case GraphicsAPI::OpenGL:
			return std::make_shared<OpenGlAntialiasing>(width,height);
		case GraphicsAPI::None:
			return nullptr;
		default:
			return std::make_shared<OpenGlAntialiasing>(width,height);//default renderer is opengl
		}
	}
	glm::vec2 Antialiasing::GetJitterOffset()
	{
		auto halton = [](int base, int index)
		{
			float result = 0.;
			float f = 1.;
			while (index > 0)
			{
				f = f / float(base);
				result += f * float(index % base);
				index = index / base;
				//index = int(floor(float(index) / float(base)));
			}
			return result;
		};
		++frameCount;
		auto res = RenderCommand::GetViewportSize(); //get screen resolution
		glm::vec2 jitter = glm::vec2(halton(2, frameCount), halton(3, frameCount));
		jitter = ((jitter - 0.5f)/res) * 2.0f; //convert from -1 to +1
		//jitter /= res; //jitter in pixel level
		return jitter;
	}
}