#pragma once
namespace Hazel {
	enum class GraphicsAPI
	{
		None=0,
		OpenGL=1
	};
	class Renderer {
	public:
		static GraphicsAPI GetAPI() { return m_API; }
	private:
		
		static GraphicsAPI m_API;
	};
}