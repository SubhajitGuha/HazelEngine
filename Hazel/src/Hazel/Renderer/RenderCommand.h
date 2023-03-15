#pragma once
#include "RendererAPI.h"
#include "Hazel/Core.h"

namespace Hazel {
	class RenderCommand {
	public:
		inline static void Init() { m_RendererAPI->Init(); }

		static void SetViewport(unsigned int Width, unsigned int Height);

		inline static void ClearColor(const glm::vec4& color) {
			m_RendererAPI->ClearColor(color);
		}
		inline static void Clear() {
			m_RendererAPI->Clear();
		}
		inline static void DrawIndex(VertexArray& vertexarray) {
			m_RendererAPI->DrawIndex(vertexarray);
		}
		inline static void DrawArrays(VertexArray& vertexarray,size_t count, int first = 0)
		{
			m_RendererAPI->DrawArrays(vertexarray,count,first);
		}
		inline static void DrawLine(VertexArray& vertexarray,uint32_t& count) {
			m_RendererAPI->DrawLine(vertexarray,count);
		}
		inline static glm::vec2 GetViewportSize()
		{
			return m_RendererAPI->GetViewportSize();
		}
	private:
		static ref<RendererAPI> GetRendererAPI();
		static ref<RendererAPI> m_RendererAPI;
	};
}