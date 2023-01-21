#pragma once
#include "Hazel/Renderer/FrameBuffer.h"

namespace Hazel {
	class OpenGlFrameBuffer : public FrameBuffer
	{
	public:
		OpenGlFrameBuffer(const FrameBufferSpecification& spec);
		~OpenGlFrameBuffer();
		virtual unsigned int GetSceneTextureID() override { return m_SceneTexture; }
		virtual unsigned int GetDepthTextureID() override { return m_DepthTexture; }
		virtual FrameBufferSpecification GetSpecification() override { return Specification; }
		virtual void Bind() override;
		virtual void UnBind() override;
		virtual void Resize(unsigned int width, unsigned int height) override;
		virtual void ClearFrameBuffer()override;
	private:
		unsigned int m_RenderID=0,m_SceneTexture=0,m_DepthTexture=0;
		FrameBufferSpecification Specification;
	private:
		void invalidate(const FrameBufferSpecification& spec);
	};
}

