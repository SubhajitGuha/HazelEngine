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
	void OpenGlRendererAPI::ClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
	void OpenGlRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void OpenGlRendererAPI::DrawIndex(VertexArray& vertexarray)
	{
		vertexarray.Bind();
		glDrawElements(GL_TRIANGLES, vertexarray.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, (const void *)0);
	}
	void OpenGlRendererAPI::DrawLine(VertexArray& vertexarray,uint32_t count)
	{
		
		vertexarray.Bind();
		glDrawArrays(GL_LINES,0,count);
	}
	void OpenGlRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}
	void OpenGlRendererAPI::SetViewPort(unsigned int Width, unsigned int Height)
	{
		glViewport(0, 0, Width, Height);
	}
}