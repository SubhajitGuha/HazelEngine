#pragma once
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "glm/glm.hpp"
#include "Shader.h"

namespace Hazel {
	class Renderer {
	public:
		static void BeginScene(OrthographicCamera& camera) 
		{ 
			m_data->m_ProjectionViewMatrix = camera.GetProjectionViewMatix(); 
		}
		static void Submit(Shader& shader , VertexArray& vertexarray , glm::mat4 ModelTransform = glm::mat4(1))
		{
			shader.Bind();
			shader.UploadUniformMat4("m_ProjectionView", Renderer::m_data->m_ProjectionViewMatrix);
			shader.UploadUniformMat4("m_ModelTransform", ModelTransform);

			vertexarray.Bind();
			RenderCommand::DrawIndex(vertexarray);
		}
		static void EndScene(){}

		struct data {
			glm::mat4 m_ProjectionViewMatrix;
		};
		static data* m_data;

	
	};
}