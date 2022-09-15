#include "hzpch.h"
#include "RenderCommand.h"
#include "Hazel/platform/Opengl/OpenGlRendererAPI.h"
#include "Hazel/Log.h"

namespace Hazel {
	
	ref<RendererAPI> RenderCommand::m_RendererAPI = GetRendererAPI();

		void RenderCommand::SetViewport(unsigned int Width, unsigned int Height)
		{
			m_RendererAPI->SetViewPort(Width, Height);
		}

		ref<RendererAPI> RenderCommand::GetRendererAPI()
		{
			switch (RendererAPI::GetAPI()) {
			case GraphicsAPI::None:
				return nullptr;
			case GraphicsAPI::OpenGL:
				return std::make_shared<OpenGlRendererAPI>();
			default:
				HAZEL_CORE_ERROR("No valid Graphics api");
				return nullptr;
			}
		}
}