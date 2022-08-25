#include "hzpch.h"
#include "OpenGlRendererAPI.h"
#include "glad/glad.h"

namespace Hazel {
	OpenGlRendererAPI::OpenGlRendererAPI()
	{
	}
	OpenGlRendererAPI::~OpenGlRendererAPI()
	{
	}
	void OpenGlRendererAPI::ClearColor(glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
	void OpenGlRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
	void OpenGlRendererAPI::DrawIndex(unsigned int NumberOfIndex,unsigned int Offset)
	{
		glDrawElements(GL_TRIANGLES, NumberOfIndex, GL_UNSIGNED_INT, (const void *)Offset);
	}
}