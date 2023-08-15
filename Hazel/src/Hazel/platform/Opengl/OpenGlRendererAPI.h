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
		void DrawArrays(VertexArray& vertexarray,size_t count, int first = 0) override;
		void DrawArrays(VertexArray& vertexarray, size_t count, unsigned int renderingMode, int first) override;
		virtual void DrawInstancedArrays(VertexArray& vertexarray, size_t count, size_t instance_count, int first = 0) override;
		void DrawLine(VertexArray& vertexarray,uint32_t count)override;
		void Init() override;
		void SetViewPort(unsigned int, unsigned int) override;
		virtual glm::vec2 GetViewportSize() override;
	};
}