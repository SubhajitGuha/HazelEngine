#include "hzpch.h"
#include "Renderer.h"

namespace Hazel {
	Renderer::data* Renderer::m_data = new Renderer::data;
	void Renderer::Init()
	{
		RenderCommand::Init();
	}
	void Renderer::WindowResize(unsigned int Width, unsigned int Height)
	{
		RenderCommand::SetViewport(Width, Height);
	}
	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_data->m_ProjectionViewMatrix = camera.GetProjectionViewMatix();
	}
	void Renderer::Submit(Shader& shader, VertexArray& vertexarray, glm::mat4 ModelTransform)
	{
		shader.Bind();
		shader.SetMat4("m_ProjectionView", Renderer::m_data->m_ProjectionViewMatrix);
		shader.SetMat4("m_ModelTransform", ModelTransform);

		vertexarray.Bind();
		RenderCommand::DrawIndex(vertexarray);
	}
}