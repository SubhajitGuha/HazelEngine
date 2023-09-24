#include "Hazel.h"

namespace Hazel
{
	class DefferedRenderer
	{
	public:
		static void Init(int width, int height);
		static void GenerateGBuffers(Scene*);
		static void DeferredRenderPass();
		static ref<Shader> GetDeferredPassShader();
		static uint32_t GetBuffers(int bufferInd);
	};
}