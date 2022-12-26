#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
#include "Hazel/Renderer/Buffer.h"

namespace Hazel {
	enum class GraphicsAPI
	{
		None = 0,
		OpenGL = 1
	};
	class RendererAPI {
	public:
		virtual void ClearColor(const glm::vec4&) = 0;
		virtual void Clear() =0;
		virtual void DrawIndex(VertexArray& vertexarray) =0;
		virtual void DrawLine(VertexArray& vertexarray, uint32_t count) = 0;
		inline static GraphicsAPI GetAPI() { return m_API; }
		virtual void Init() = 0;
		virtual void SetViewPort(unsigned int, unsigned int) = 0;
	private:
		static GraphicsAPI m_API;
	};
}