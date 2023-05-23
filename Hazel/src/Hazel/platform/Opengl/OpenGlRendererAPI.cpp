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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	void OpenGlRendererAPI::DrawIndex(VertexArray& vertexarray)
	{
		vertexarray.Bind();
		glDrawElements(GL_TRIANGLES, vertexarray.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, (const void*)0);
	}
	void OpenGlRendererAPI::DrawArrays(VertexArray& vertexarray,size_t count,int first)
	{
		vertexarray.Bind();
		glDrawArrays(GL_TRIANGLES, first, count);
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
		//glEnable(GL_STENCIL_TEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	void OpenGlRendererAPI::SetViewPort(unsigned int Width, unsigned int Height)
	{
		glViewport(0, 0, Width, Height);
	}
	glm::vec2 OpenGlRendererAPI::GetViewportSize()
	{
		float arr[4];
		glGetFloatv(GL_VIEWPORT, arr);
		return {arr[2],arr[3]};//the index 2 and 3 gives the width and height of the viewport
	}
}