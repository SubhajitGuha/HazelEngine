#include "hzpch.h"
#include "OpenGlFrameBuffer.h"
#include "glad/glad.h"
#include "Hazel/Log.h"

namespace Hazel {
    OpenGlFrameBuffer::OpenGlFrameBuffer(const FrameBufferSpecification& spec)
    {
        glGenFramebuffers(1, &m_RenderID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RenderID);

        glGenTextures(1, &m_SceneTexture);
        glBindTexture(GL_TEXTURE_2D, m_SceneTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, spec.Width, spec.Height);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, spec.Width, spec.Height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,m_SceneTexture,0);

        glGenTextures(1, &m_DepthTexture);
        glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, spec.Width, spec.Height);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, spec.Width, spec.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
            HAZEL_CORE_INFO("FrameBuffer Compleate");
        else
            HAZEL_CORE_ERROR("FrameBuffer inCompleate !!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    OpenGlFrameBuffer::~OpenGlFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_RenderID);
    }
    void OpenGlFrameBuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RenderID);
    }
    void OpenGlFrameBuffer::UnBind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}