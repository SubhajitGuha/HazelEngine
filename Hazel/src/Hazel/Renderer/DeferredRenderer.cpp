#include "hzpch.h"
#include "Hazel/platform/Opengl/OpenGlDeferredRenderer.h"
#include "DeferredRenderer.h"

namespace Hazel
{
	void DefferedRenderer::Init(int width, int height)
	{
		OpenGlDeferredRenderer::Init(width,height);
	}
	void DefferedRenderer::GenerateGBuffers(Scene* scene)
	{
		OpenGlDeferredRenderer::CreateBuffers(scene);
	}
	void DefferedRenderer::DeferredRenderPass()
	{
		OpenGlDeferredRenderer::DeferredPass();
	}
	ref<Shader> DefferedRenderer::GetDeferredPassShader()
	{
		return OpenGlDeferredRenderer::GetDeferredShader();
	}
	uint32_t DefferedRenderer::GetBuffers(int bufferInd)
	{
		return OpenGlDeferredRenderer::GetBuffers(bufferInd);
	}
}