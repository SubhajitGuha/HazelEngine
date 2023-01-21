#pragma once
#include "Hazel/Core.h"
namespace Hazel {
		struct FrameBufferSpecification {
			unsigned int Width, Height;
			bool SwapChainTarget = false;
		};
	class FrameBuffer
	{
	public:
		static ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
		virtual FrameBufferSpecification GetSpecification() = 0;
		virtual void Bind()=0;
		virtual void UnBind()=0;
		virtual unsigned int GetSceneTextureID() = 0;
		virtual unsigned int GetDepthTextureID() = 0;
		virtual void Resize(unsigned int width, unsigned int height) = 0;
		virtual void ClearFrameBuffer() = 0;
	};
}
