#include "Hazel.h"

namespace Hazel
{
	class OpenGlDeferredRenderer
	{
	public:
		static void Init(int width, int height);
		static void CreateBuffers(Scene* scene);
		static void DeferredPass();
		static ref<Shader> GetDeferredShader() { return m_DefferedPassShader; }
		static uint32_t GetBuffers(int bufferInd);

	private:
		static uint32_t m_framebufferID, m_RenderBufferID, m_AlbedoBufferID, m_NormalBufferID , m_PositionBufferID, m_RoughnessMetallicBufferID;
		static ref<Shader> m_ForwardPassShader;
		static ref<Shader> m_DefferedPassShader;
	};
}