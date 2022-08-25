#pragma once
#include "RendererAPI.h"

namespace Hazel {
	class RenderCommand {
	public:
		inline static void ClearColor(glm::vec4& color) {
			m_RendererAPI->ClearColor(color);
		}
		inline static void Clear() {
			m_RendererAPI->Clear();
		}
		inline static void DrawIndex(const unsigned int& NumberOfIndex,const unsigned int& StartIndex) {
			m_RendererAPI->DrawIndex(NumberOfIndex, StartIndex);
		}
	private:
		static RendererAPI* m_RendererAPI;
	};
}