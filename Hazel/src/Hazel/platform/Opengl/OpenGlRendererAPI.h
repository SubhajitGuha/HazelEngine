#pragma once
#include "Hazel/Renderer/RendererAPI.h"
namespace Hazel {
	class OpenGlRendererAPI: public RendererAPI
	{
	public:
		OpenGlRendererAPI();
		~OpenGlRendererAPI();
		void ClearColor(glm::vec4&)override;
		void Clear()override;
		void DrawIndex(VertexArray& vertexarray)override;
	};
}