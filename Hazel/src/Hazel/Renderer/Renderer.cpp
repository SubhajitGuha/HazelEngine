#include "hzpch.h"
#include "Renderer.h"

namespace Hazel {
	Renderer::data* Renderer::m_data = new Renderer::data;
	void Renderer::Init()
	{
		RenderCommand::Init();
	}
}