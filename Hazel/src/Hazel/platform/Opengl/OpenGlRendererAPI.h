#pragma once
#include "Hazel/Renderer/RendererAPI.h"
namespace Hazel {
	class OpenGlRendererAPI: public RendererAPI
	{
	public:
		OpenGlRendererAPI();
		~OpenGlRendererAPI();
		void ClearColor(const glm::vec4&)override;
		void Clear()override;
		void DrawIndex(VertexArray& vertexarray)override;
		void DrawLine(VertexArray& vertexarray,uint32_t count)override;
		void Init() override;
		void SetViewPort(unsigned int, unsigned int) override;
	};
}