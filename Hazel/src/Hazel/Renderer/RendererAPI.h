#pragma once
#include "Hazel/Log.h"
#include "glm/glm.hpp"
namespace Hazel {
	enum class GraphicsAPI
	{
		None = 0,
		OpenGL = 1
	};
	class RendererAPI {
	public:
		virtual void ClearColor(glm::vec4&) = 0;
		virtual void Clear() =0;
		virtual void DrawIndex(unsigned int NumberOfIndex, unsigned int Offset) =0;
		inline static GraphicsAPI GetAPI() { return m_API; }
	private:
		static GraphicsAPI m_API;
	};
}