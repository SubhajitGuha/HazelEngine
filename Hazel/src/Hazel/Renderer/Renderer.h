#pragma once
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "glm/glm.hpp"
#include "Shader.h"

namespace Hazel {
	class Renderer {
	public:
		~Renderer() { delete m_data; }
		static void Init();
		static void WindowResize(unsigned int Width, unsigned int Height); 
		static void BeginScene(OrthographicCamera& camera);
		static void Submit(Shader& shader, VertexArray& vertexarray, glm::mat4 ModelTransform = glm::mat4(1));
		static void EndScene(){}

		struct data {
			glm::mat4 m_ProjectionViewMatrix;
		};
		static data* m_data;	
	};
}