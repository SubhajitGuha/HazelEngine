#include "hzpch.h"
#include "RenderCommand.h"
#include "Hazel/platform/Opengl/OpenGlRendererAPI.h"

namespace Hazel {
		RendererAPI* RenderCommand::m_RendererAPI = new OpenGlRendererAPI();
	}