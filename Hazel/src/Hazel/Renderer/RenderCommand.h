#pragma once
#include "RendererAPI.h"

namespace Hazel {
	class RenderCommand {
	public:
		inline static void Init() { m_RendererAPI->Init(); }
		inline static void ClearColor(const glm::vec4& color) {
			m_RendererAPI->ClearColor(color);
		}
		inline static void Clear() {
			m_RendererAPI->Clear();
		}
		inline static void DrawIndex(VertexArray& vertexarray) {
			m_RendererAPI->DrawIndex(vertexarray);
		}
	private:
		static RendererAPI* m_RendererAPI;
	};
}