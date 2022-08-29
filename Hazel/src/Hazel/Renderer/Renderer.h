#pragma once
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "glm/glm.hpp"
#include "Shader.h"
namespace Hazel {
	class Renderer {
	public:
		static void BeginScene(OrthographicCamera& camera) { m_data->m_ProjectionViewMatrix = camera.GetProjectionViewMatix();}
		static void Submit(VertexArray& vertexarray) {RenderCommand::DrawIndex(vertexarray);}
		static void EndScene(){}

		struct data {
			glm::mat4 m_ProjectionViewMatrix;
		};
		static data* m_data;
	};
}