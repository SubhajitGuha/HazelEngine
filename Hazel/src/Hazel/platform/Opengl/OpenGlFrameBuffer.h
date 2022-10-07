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
	private:
		unsigned int m_RenderID,m_SceneTexture,m_DepthTexture;
		FrameBufferSpecification Specification;
	};
}

